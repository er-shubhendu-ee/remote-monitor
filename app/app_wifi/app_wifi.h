/**
 * @file      app_wifi.h
 * @author:   Shubhendu B B
 * @date:     28/09/2025
 * @brief     Wi-Fi module interface (AP, STA, AP+STA)
 * @details   Portable header with no target-dependent includes
 */

#ifndef _APP_WIFI_H_
#define _APP_WIFI_H_

#include <stdbool.h>
#include <stdint.h>

#define APP_WIFI_SSID_SIZE_MAX 32
#define APP_WIFI_PASS_SIZE_MAX 64
#define APP_WIFI_IP_SIZE_MAX 16

typedef enum {
    APP_WIFI_EVENT_TYPE_INVALID = 0,

    // Output
    APP_WIFI_EVENT_TYPE_OUT_STA_GOT_IP,

    APP_WIFI_EVENT_TYPE_MAX = 0xFF
} app_wifi_EventType_t;

typedef struct {
    app_wifi_EventType_t eventType;
    union {
        struct {
            uint8_t* pBuff;
            uint32_t buffSize;
        } dataBuff;
    } param;
} app_wifi_EventMessage_t;

typedef struct {
    bool ap;
    bool sta;
} app_wifi_FeatureType_t;

typedef struct {
    struct {
        char ssidTarget[APP_WIFI_SSID_SIZE_MAX + 1];
        char passTarget[APP_WIFI_PASS_SIZE_MAX + 1];
        char ipAssigned[APP_WIFI_IP_SIZE_MAX + 1];
    } sta;
    struct {
        char ssidTarget[APP_WIFI_SSID_SIZE_MAX + 1];
        char passTarget[APP_WIFI_PASS_SIZE_MAX + 1];
        char ipTarget[APP_WIFI_IP_SIZE_MAX + 1];
        uint8_t channel;
        uint8_t connectionMax;
    } ap;
} app_wifi_config_t;

typedef void (*app_wifi_EventCb_t)(app_wifi_EventMessage_t* pEventMessage, int isFromIsr);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Wi-Fi module
 */
int app_wifi_Init(app_wifi_EventCb_t eventCb);

/**
 * @brief Start Access Point
 * @param ssid     Null-terminated SSID string
 * @param password Null-terminated password string (optional, can be NULL)
 * @param max_conn Maximum number of clients
 */
int app_wifi_Start(const app_wifi_config_t* pInitConfig);

/**
 * @brief Stop Wi-Fi (both AP and STA)
 */
int app_wifi_Stop(void);

/**
 * @brief Get current Wi-Fi status
 */
int app_wifi_GetStatus(app_wifi_FeatureType_t feature, const app_wifi_config_t* pInitConfig);

#ifdef __cplusplus
}
#endif

#endif /* _APP_WIFI_H_ */
