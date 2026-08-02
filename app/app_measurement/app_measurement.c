/**
 * @file      app_measurement.c
 * @author:   Shubhendu B B
 * @date:     02/08/2026
 * @brief
 * @details   Distributed globally for free under the MIT License terms.
 *
 * @copyright Copyright (c) 2025 er-shubhendu-ee
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/

#include "util_common_error.h"  // Framework error definitions (NO_ERROR, etc.)
#include "util_common_log.h"    // Framework logger tool architecture definitions

// Private system dependencies
#include "ads1115.h"
#include "app_measurement.h"
#include "driver/i2c_master.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "pf8591.h"

#define LOG_LEVEL LOG_LEVEL_DEBUG
#define TAG "APP_MEASUREMENT"

// Private module tracking states
static app_measurement_cb_t g_measurement_cb = NULL;
static ads1115_handle_t g_adc_device = NULL;
static esp_timer_handle_t g_ads_timer = NULL;
static i2c_master_bus_handle_t g_i2c_bus_handle = NULL;
static bool g_is_initialized = false;

int app_measurement_PostEvent(app_measurement_EventMessage_t* pEventMessage, bool isFromIsr) {
    if (g_measurement_cb != NULL) {
        g_measurement_cb(pEventMessage, isFromIsr ? 1 : 0);
        return NO_ERROR;  // Unified template mapping
    }
    return ERROR_INVALID_ACCESS;
}

static void ads1115_timer_callback(void* arg) {
    ads1115_handle_t adc_device = (ads1115_handle_t)arg;
    int16_t raw_result;
    app_measurement_EventMessage_t outEvent = {0};

    esp_err_t ret = ads1115_read_differential_0_1(adc_device, &raw_result);

    if (ret == ESP_OK) {
        outEvent.eventType = APP_MEASUREMENT_EVENT_TYPE_OUT_DATA_READY;
        outEvent.param.measurementData.rawCode = raw_result;
        outEvent.param.measurementData.voltage =
            ads1115_compute_volts(ADS1115_PGA_4_096V, raw_result);
        outEvent.param.measurementData.errorStatus = (int32_t)NO_ERROR;
    } else {
        outEvent.eventType = APP_MEASUREMENT_EVENT_TYPE_OUT_ERROR;
        outEvent.param.measurementData.rawCode = 0;
        outEvent.param.measurementData.voltage = 0.0f;
        outEvent.param.measurementData.errorStatus = (int32_t)ret;
#if LOG_LEVEL >= LOG_LEVEL_ERROR
        SERVICE_LOGE(TAG, "I2C Read Error: %d", ret);
#endif
    }

    // esp_timer context handles non-blocking callback hooks safely directly
    app_measurement_PostEvent(&outEvent, false);
}

int app_measurement_Init(app_measurement_Configuration_t* pConfiguration,
                         app_measurement_cb_t eventCb) {
    if (g_is_initialized) {
#if LOG_LEVEL >= LOG_LEVEL_WARN
        SERVICE_LOGW(TAG, "Already initialized.");
#endif
        return ERROR_ACCESS_DENIED;
    }

    if (pConfiguration == NULL || eventCb == NULL) {
#if LOG_LEVEL >= LOG_LEVEL_ERROR
        SERVICE_LOGE(TAG, "Invalid configurations passed.");
#endif
        return ERROR_INVALID_PARAMETER;
    }

    g_measurement_cb = eventCb;

    // Set up master I2C architecture bus mapping
    i2c_master_bus_config_t bus_config = {.clk_source = I2C_CLK_SRC_DEFAULT,
                                          .i2c_port = I2C_NUM_0,
                                          .scl_io_num = pConfiguration->i2cSclIoNum,
                                          .sda_io_num = pConfiguration->i2cSdaIoNum,
                                          .glitch_filter_blink_num = 7};
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &g_i2c_bus_handle));

    // Initialize the external ADS1115 peripheral device register cache
    ads1115_config_t adc_config = {.bus_handle = g_i2c_bus_handle,
                                   .i2c_address = ADS1115_ADDR_GND,
                                   .pga_gain = ADS1115_PGA_4_096V,
                                   .data_rate = ADS1115_SPS_860,
                                   .mode = ADS1115_MODE_CONTINUOUS};
    ESP_ERROR_CHECK(ads1115_init(&adc_config, &g_adc_device));

    // Setup periodic ms hardware callback timer
    const esp_timer_create_args_t ads_timer_args = {.callback = &ads1115_timer_callback,
                                                    .arg = (void*)g_adc_device,
                                                    .name = "ads1115_timer",
                                                    .skip_unhandled_events = true};
    ESP_ERROR_CHECK(esp_timer_create(&ads_timer_args, &g_ads_timer));

    // Convert initialization millisecond input units safely to microseconds
    uint64_t timeout_period_us = pConfiguration->timeoutPeriodMs * 1000ULL;
    ESP_ERROR_CHECK(esp_timer_start_periodic(g_ads_timer, timeout_period_us));

    g_is_initialized = true;
#if LOG_LEVEL >= LOG_LEVEL_INFO
    SERVICE_LOGI(TAG, "Measurement module started successfully.");
#endif
    return NO_ERROR;
}

int app_measurement_DeInit(void) {
    if (!g_is_initialized) {
        return ERROR_ACCESS_DENIED;
    }

    if (g_ads_timer != NULL) {
        esp_timer_stop(g_ads_timer);
        esp_timer_delete(g_ads_timer);
        g_ads_timer = NULL;
    }

    if (g_i2c_bus_handle != NULL) {
        i2c_del_master_bus(g_i2c_bus_handle);
        g_i2c_bus_handle = NULL;
    }

    g_measurement_cb = NULL;
    g_adc_device = NULL;
    g_is_initialized = false;

#if LOG_LEVEL >= LOG_LEVEL_INFO
    SERVICE_LOGI(TAG, "Measurement module stopped and de-initialized.");
#endif
    return NO_ERROR;
}
