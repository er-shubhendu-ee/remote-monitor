/**
 * @file      app_server.h
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

#ifndef _APP_SERVER_H_
#define _APP_SERVER_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    APP_SERVER_EVENT_TYPE_UNDEFINED = 0,

    // input
    APP_SERVER_EVENT_TYPE_IN_START_LISTENING,
    APP_SERVER_EVENT_TYPE_IN_STOP_LISTENING,

    // output
    APP_SERVER_EVENT_TYPE_OUT_SERVER_STARTED,
    APP_SERVER_EVENT_TYPE_OUT_LISTENING_STARTED,
    APP_SERVER_EVENT_TYPE_OUT_LISTENING_START_ERROR,
    APP_SERVER_EVENT_TYPE_OUT_LISTENING_STOPPED,
    APP_SERVER_EVENT_TYPE_OUT_LISTENING_STOP_ERROR,
    APP_SERVER_EVENT_TYPE_OUT_CONTROL_COMMAND,

    APP_SERVER_EVENT_TYPE_MAX = 0xFF,
} app_server_eventType_t;

typedef struct {
    uint8_t ip[16];
    uint16_t port;
    uint8_t ipV6Scope;
    uint8_t isIpV6 : 1;
} app_server_Address_t;

typedef struct {
    app_server_eventType_t eventType;
    union {
        struct {
            uint8_t* pData;
            uint32_t dataSize;
        } messageData;
        app_server_Address_t listenData;
    } param;
} app_server_EventMessage_t;

typedef struct {
    char* pStorageRootPath;
    uint32_t storageRootPathLen;
    char* pHostIp;
    uint32_t hostIpLen;
} app_server_Configuration_t;

typedef void (*app_server_cb_t)(app_server_EventMessage_t* pEventMessage, int isFromIsr);

#ifdef __cplusplus
extern "C" {
#endif

int app_server_Init(app_server_Configuration_t* pConfiguration, app_server_cb_t eventCb);
int app_server_DeInit(void);
int app_server_PostEvent(app_server_EventMessage_t* pEventMessage, bool isFromIsr);

#ifdef __cplusplus
}
#endif

#endif /* @end  _APP_SERVER_H_*/