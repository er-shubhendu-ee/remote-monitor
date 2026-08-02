/**
 * @file      app_wifi.c
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

#include "util_common_error.h"
#include "util_common_log.h"

//
#include "config_app.h"
#include "app_wifi.h"

//
#ifndef CONFIG_LWIP_IPV4_NAPT
#define CONFIG_LWIP_IPV4_NAPT
#endif

//
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#if IP_NAPT
#include "lwip/lwip_napt.h"
#endif
#include "lwip/err.h"
#include "lwip/sys.h"

//
#include "nvs_flash.h"

//
#include "esp_event.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_netif_net_stack.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"

//
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"

//
#include <stdio.h>
#include <string.h>

#define LOG_LEVEL LOG_LEVEL_DEBUG
#define TAG "APP_WIFI"

/* STA Configuration */
// #define EXAMPLE_ESP_WIFI_STA_SSID "connect-here"
// #define EXAMPLE_ESP_WIFI_STA_PASSWD "9876543210"
// #define EXAMPLE_ESP_MAXIMUM_RETRY 10

#define CONFIG_ESP_WIFI_AUTH_WPA2_PSK 1

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* AP Configuration */
// #define EXAMPLE_ESP_WIFI_AP_SSID "shubho-server"
// #define EXAMPLE_ESP_WIFI_AP_PASSWD "9876543210"
// #define EXAMPLE_ESP_WIFI_CHANNEL 1
// #define EXAMPLE_MAX_STA_CONN 2

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

/*DHCP server option*/
#define DHCPS_OFFER_DNS 0x02

typedef enum tagEventType {
    EVENT_TYPE_UNDEFINED = 0,  //
    EVENT_TYPE_WIFI,
    EVENT_TYPE_IP,
    EVENT_TYPE_MAX
} eventType_t;

static int s_retry_num = 0;

//
app_wifi_EventCb_t gEventCb;

/* FreeRTOS event group to signal when we are connected/disconnected */
static app_wifi_config_t gWifiConfig;
static esp_netif_t* gNetIf_sta;
static esp_netif_t* gNetIf_ap;
static EventGroupHandle_t s_wifi_event_group;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id,
                               void* event_data);
static esp_netif_t* wifi_init_sta(wifi_config_t* pApConfig);
static esp_netif_t* wifi_init_softap(wifi_config_t* pApConfig);
static void softap_set_dns_addr(esp_netif_t* gNetIf_ap, esp_netif_t* gNetIf_sta);

int app_wifi_Init(app_wifi_EventCb_t eventCb) {
    if (!eventCb) {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGE(TAG, "NULL argument.");
#endif
        return ERROR_BAD_ARGUMENTS;
    }

    gEventCb = eventCb;

    ESP_ERROR_CHECK(esp_netif_init());

    /* Initialize event group */
    s_wifi_event_group = xEventGroupCreate();

    /* Register Event handler */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler, NULL, NULL));

    return 0;
}

int app_wifi_Start(const app_wifi_config_t* pInitConfig) {
    if (!pInitConfig) {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGE(TAG, "NULL argument.");
#endif
        return ERROR_BAD_ARGUMENTS;
    }

    if (!strlen(pInitConfig->sta.ssidTarget) || !strlen(pInitConfig->sta.passTarget)) {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGE(TAG, "STA information invalid");
#endif
        return ERROR_INVALID_DATA;
    }

    if (!strlen(pInitConfig->ap.ssidTarget) || !strlen(pInitConfig->ap.passTarget) ||
        !pInitConfig->ap.connectionMax) {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGE(TAG, "AP information invalid");
#endif
        return ERROR_INVALID_DATA;
    }

    uint32_t targetSize = 0;
    uint32_t sourceSize = 0;
    char ssid[APP_WIFI_SSID_SIZE_MAX + 1];
    char pass[APP_WIFI_PASS_SIZE_MAX + 1];

    memset(ssid, 0, sizeof(ssid));
    memset(pass, 0, sizeof(pass));

    /* STA */
    /* SSID length check */
    if (APP_WIFI_SSID_SIZE_MAX < strlen(pInitConfig->sta.ssidTarget)) {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGW(TAG, "STA SSID length %d longer than maximum %d",
                     strlen(pInitConfig->sta.ssidTarget), APP_WIFI_SSID_SIZE_MAX);
#endif
    }

    /* Password length check */
    if (APP_WIFI_PASS_SIZE_MAX < strlen(pInitConfig->sta.passTarget)) {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGE(TAG, "STA Password length %d longer than maximum %d",
                     strlen(pInitConfig->sta.passTarget), APP_WIFI_PASS_SIZE_MAX);
#endif
        return ERROR_INVALID_DATA;
    }

    /* AP */
    /* SSID length check */
    if (APP_WIFI_SSID_SIZE_MAX < strlen(pInitConfig->ap.ssidTarget)) {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGW(TAG, "STA SSID length %d longer than maximum %d",
                     strlen(pInitConfig->ap.ssidTarget), APP_WIFI_SSID_SIZE_MAX);
#endif
    }

    /* Password length check */
    if (APP_WIFI_PASS_SIZE_MAX < strlen(pInitConfig->ap.passTarget)) {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGE(TAG, "STA Password length %d longer than maximum %d",
                     strlen(pInitConfig->ap.passTarget), APP_WIFI_PASS_SIZE_MAX);
#endif
        return ERROR_INVALID_DATA;
    }

    /* Copy information to global */
    /* STA */
    strncpy(gWifiConfig.sta.ssidTarget, pInitConfig->sta.ssidTarget,
            (APP_WIFI_SSID_SIZE_MAX < strlen(pInitConfig->sta.ssidTarget))
                ? APP_WIFI_SSID_SIZE_MAX
                : strlen(pInitConfig->sta.ssidTarget));
    strncpy(gWifiConfig.sta.passTarget, pInitConfig->sta.passTarget,
            (APP_WIFI_PASS_SIZE_MAX < strlen(pInitConfig->sta.passTarget))
                ? APP_WIFI_PASS_SIZE_MAX
                : strlen(pInitConfig->sta.passTarget));

    /* AP */
    strncpy(gWifiConfig.ap.ssidTarget, pInitConfig->ap.ssidTarget,
            (APP_WIFI_SSID_SIZE_MAX < strlen(pInitConfig->ap.ssidTarget))
                ? APP_WIFI_SSID_SIZE_MAX
                : strlen(pInitConfig->ap.ssidTarget));
    strncpy(gWifiConfig.ap.passTarget, pInitConfig->ap.passTarget,
            (APP_WIFI_PASS_SIZE_MAX < strlen(pInitConfig->ap.passTarget))
                ? APP_WIFI_PASS_SIZE_MAX
                : strlen(pInitConfig->ap.passTarget));

    /*Initialize WiFi */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    /* Initialize STA */
    wifi_config_t wifiConfig_sta;
    memset(&wifiConfig_sta, 0, sizeof(wifiConfig_sta));

    /* copy ssid */
    sourceSize = strlen(pInitConfig->sta.ssidTarget);
    targetSize = sizeof(wifiConfig_sta.sta.ssid);
    memcpy(wifiConfig_sta.sta.ssid, pInitConfig->sta.ssidTarget,
           (targetSize < sourceSize) ? targetSize : sourceSize);

    /* copy password */
    sourceSize = strlen(pInitConfig->sta.passTarget);
    targetSize = sizeof(wifiConfig_sta.sta.password);
    memcpy(wifiConfig_sta.sta.password, pInitConfig->sta.passTarget,
           (targetSize < sourceSize) ? targetSize : sourceSize);

    wifiConfig_sta.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifiConfig_sta.sta.failure_retry_cnt = APP_CONFIG_STA_RETRY_CNT_MAX;
    wifiConfig_sta.sta.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD;
    wifiConfig_sta.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

#if LOG_LEVEL >= LOG_LEVEL_INFO
    SERVICE_LOGI(TAG, "ESP_WIFI_MODE_STA");
#endif
    gNetIf_sta = wifi_init_sta(&wifiConfig_sta);

    /* Initialize AP */
    wifi_config_t wifiConfig_ap;
    memset(&wifiConfig_ap, 0, sizeof(wifiConfig_ap));

    /* copy ssid */
    sourceSize = strlen(pInitConfig->ap.ssidTarget);
    targetSize = sizeof(wifiConfig_ap.ap.ssid);
    memcpy(wifiConfig_ap.ap.ssid, pInitConfig->ap.ssidTarget,
           (targetSize < sourceSize) ? targetSize : sourceSize);
    wifiConfig_ap.ap.ssid_len = sourceSize;

    /* copy password */
    wifiConfig_ap.ap.authmode = WIFI_AUTH_WPA2_PSK;
    sourceSize = strlen(pInitConfig->ap.passTarget);
    targetSize = sizeof(wifiConfig_ap.ap.password);
    memcpy(wifiConfig_ap.ap.password, pInitConfig->ap.passTarget,
           (targetSize < sourceSize) ? targetSize : sourceSize);

    wifiConfig_ap.ap.channel = pInitConfig->ap.channel;
    wifiConfig_ap.ap.max_connection = pInitConfig->ap.connectionMax;
    wifiConfig_ap.ap.pmf_cfg.required = false;

    if (0 == sourceSize) {
        wifiConfig_ap.ap.authmode = WIFI_AUTH_OPEN;
    }

#if LOG_LEVEL >= LOG_LEVEL_INFO
    SERVICE_LOGI(TAG, "ESP_WIFI_MODE_AP");
#endif
    gNetIf_ap = wifi_init_softap(&wifiConfig_ap);

    /* Start WiFi */
    ESP_ERROR_CHECK(esp_wifi_start());

    /*
     * Wait until either the connection is established (WIFI_CONNECTED_BIT) or
     * connection failed for the maximum number of re-tries (WIFI_FAIL_BIT).
     * The bits are set by event_handler() (see above)
     */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, pdFALSE, portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned,
     * hence we can test which event actually happened. */
    if (bits & WIFI_CONNECTED_BIT) {
#if LOG_LEVEL >= LOG_LEVEL_INFO
        SERVICE_LOGI(TAG, "connected to ap SSID:%s password:%s", gWifiConfig.sta.ssidTarget,
                     gWifiConfig.sta.passTarget);
#endif
        softap_set_dns_addr(gNetIf_ap, gNetIf_sta);
    } else if (bits & WIFI_FAIL_BIT) {
#if LOG_LEVEL >= LOG_LEVEL_INFO
        SERVICE_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", gWifiConfig.sta.ssidTarget,
                     gWifiConfig.sta.passTarget);
#endif
    } else {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGE(TAG, "UNEXPECTED EVENT");
#endif
        return 0;
    }

    /* Set sta as the default interface */
    esp_netif_set_default_netif(gNetIf_sta);

    /* Enable napt on the AP netif */
    if (esp_netif_napt_enable(gNetIf_ap) != ESP_OK) {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGE(TAG, "NAPT not enabled on the netif: %p", gNetIf_ap);
#endif
    }
    return 0;
}

int app_wifi_Stop(void) { return 0; }

int app_wifi_GetStatus(app_wifi_FeatureType_t feature, const app_wifi_config_t* pInitConfig) {
    return 0;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id,
                               void* event_data) {
    eventType_t eventType;

    if (event_base == WIFI_EVENT) {
        eventType = EVENT_TYPE_WIFI;
    } else if (event_base == IP_EVENT) {
        eventType = EVENT_TYPE_IP;
    } else {
        eventType = EVENT_TYPE_UNDEFINED;
    }

    switch (eventType) {
        case EVENT_TYPE_WIFI: {
            switch (event_id) {
                case WIFI_EVENT_AP_START: {
#if LOG_LEVEL >= LOG_LEVEL_INFO
                    SERVICE_LOGI(TAG, "AP Started.");
#endif
                    break;
                }

                case WIFI_EVENT_AP_STACONNECTED: {
                    wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*)event_data;
#if LOG_LEVEL >= LOG_LEVEL_INFO
                    SERVICE_LOGI(TAG, "Station " MACSTR " joined, AID=%d", MAC2STR(event->mac),
                                 event->aid);
#endif
                    break;
                }

                case WIFI_EVENT_AP_STADISCONNECTED: {
                    wifi_event_ap_stadisconnected_t* event =
                        (wifi_event_ap_stadisconnected_t*)event_data;
#if LOG_LEVEL >= LOG_LEVEL_INFO
                    SERVICE_LOGI(TAG, "Station " MACSTR " left, AID=%d, reason:%d",
                                 MAC2STR(event->mac), event->aid, event->reason);
#endif
                    break;
                }

                case WIFI_EVENT_STA_START: {
                    esp_wifi_connect();
#if LOG_LEVEL >= LOG_LEVEL_INFO
                    SERVICE_LOGI(TAG, "Station started");
#endif
                    break;
                }

                case WIFI_EVENT_STA_CONNECTED: {
#if LOG_LEVEL >= LOG_LEVEL_INFO
                    SERVICE_LOGI(TAG, "Station connected");
#endif

                    wifi_ap_record_t ap;

                    ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&ap));

                    SERVICE_LOGI(TAG, "SSID=%s RSSI=%d CH=%d AUTH=%d", ap.ssid, ap.rssi, ap.primary,
                                 ap.authmode);

                    break;
                }

                default: {
                    SERVICE_LOGI(TAG, "%s: undefined event_id=%ld", "WIFI", event_id);
                    break;
                }
            }

            break;
        }

        case EVENT_TYPE_IP: {
            switch (event_id) {
                case IP_EVENT_STA_GOT_IP: {
                    ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
                    s_retry_num = 0;
                    /* copy ip to global */
                    snprintf(gWifiConfig.sta.ipAssigned, APP_WIFI_IP_SIZE_MAX, IPSTR,
                             IP2STR(&event->ip_info.ip));
#if LOG_LEVEL >= LOG_LEVEL_DEBUG
                    SERVICE_LOGI(TAG, "Got IP: %s", gWifiConfig.sta.ipAssigned);
#endif
                    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                    if (gEventCb) {
                        app_wifi_EventMessage_t eventParam;
                        eventParam.eventType = APP_WIFI_EVENT_TYPE_OUT_STA_GOT_IP;
                        eventParam.param.dataBuff.pBuff = (uint8_t*)gWifiConfig.sta.ipAssigned;
                        eventParam.param.dataBuff.buffSize = strlen(gWifiConfig.sta.ipAssigned);
                        gEventCb(&eventParam, xPortInIsrContext());
                    }
                    break;
                }

                default: {
                    SERVICE_LOGI(TAG, "%s: undefined event_id=%ld", "IP", event_id);
                    break;
                }
            }

            break;
        }

        default: {
            SERVICE_LOGI(TAG, "undefined event_base=%s; event_id=%ld", event_base, event_id);
            break;
        }
    }
}

/* Initialize wifi station */
static esp_netif_t* wifi_init_sta(wifi_config_t* pApConfig) {
    esp_netif_t* pNetIf = esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, pApConfig));

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
    SERVICE_LOGI(TAG, "wifi_init_sta finished.");
#endif
    return pNetIf;
}

/* Initialize soft AP */
static esp_netif_t* wifi_init_softap(wifi_config_t* pApConfig) {
    esp_netif_t* pNetIf = esp_netif_create_default_wifi_ap();

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, pApConfig));

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
    SERVICE_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
                 pApConfig->ap.ssid, pApConfig->ap.password, pApConfig->ap.channel);
#endif

    return pNetIf;
}

static void softap_set_dns_addr(esp_netif_t* gNetIf_ap, esp_netif_t* gNetIf_sta) {
    esp_netif_dns_info_t dns;
    esp_netif_get_dns_info(gNetIf_sta, ESP_NETIF_DNS_MAIN, &dns);
    uint8_t dhcps_offer_option = DHCPS_OFFER_DNS;
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(gNetIf_ap));
    ESP_ERROR_CHECK(esp_netif_dhcps_option(gNetIf_ap, ESP_NETIF_OP_SET,
                                           ESP_NETIF_DOMAIN_NAME_SERVER, &dhcps_offer_option,
                                           sizeof(dhcps_offer_option)));
    ESP_ERROR_CHECK(esp_netif_set_dns_info(gNetIf_ap, ESP_NETIF_DNS_MAIN, &dns));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(gNetIf_ap));
}
