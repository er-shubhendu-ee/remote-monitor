/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "port_system.h"

//
#include "app.h"

//
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "sdkconfig.h"

//
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//
#include <inttypes.h>
#include <stdio.h>

void app_main(void) { app_Init(); }
