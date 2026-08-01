/**
 * @file      app_measurement.h
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

#ifndef _APP_MEASUREMENT_H_
#define _APP_MEASUREMENT_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    APP_MEASUREMENT_EVENT_TYPE_UNDEFINED = 0,

    // Input events
    APP_MEASUREMENT_EVENT_TYPE_IN_START_SAMPLING,
    APP_MEASUREMENT_EVENT_TYPE_IN_STOP_SAMPLING,

    // Output events
    APP_MEASUREMENT_EVENT_TYPE_OUT_DATA_READY,
    APP_MEASUREMENT_EVENT_TYPE_OUT_ERROR,

    APP_MEASUREMENT_EVENT_TYPE_MAX = 0xFF,
} app_measurement_eventType_t;

typedef struct {
    int16_t rawCode;
    float voltage;
    int32_t errorStatus;  // Stores mapped system codes or underlying esp_err_t
} app_measurement_DataPayload_t;

typedef struct {
    app_measurement_eventType_t eventType;
    union {
        app_measurement_DataPayload_t measurementData;
    } param;
} app_measurement_EventMessage_t;

typedef struct {
    uint64_t timeoutPeriodMs;
    uint32_t i2cSclIoNum;
    uint32_t i2cSdaIoNum;
} app_measurement_Configuration_t;

typedef void (*app_measurement_cb_t)(app_measurement_EventMessage_t* pEventMessage, int isFromIsr);

#ifdef __cplusplus
extern "C" {
#endif

int app_measurement_Init(app_measurement_Configuration_t* pConfiguration,
                         app_measurement_cb_t eventCb);
int app_measurement_DeInit(void);
int app_measurement_PostEvent(app_measurement_EventMessage_t* pEventMessage, bool isFromIsr);

#ifdef __cplusplus
}
#endif

#endif /* @end _APP_MEASUREMENT_H_ */
