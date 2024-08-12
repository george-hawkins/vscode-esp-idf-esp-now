/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_sleep.h"

#include "esp_mac.h"

#include "espnow_utils.h"

#include "espnow.h"
#include "espnow_ctrl.h"

#include "iot_button.h"

#define BULB_STATUS_KEY       "bulb_key"

static const char *TAG = "app_switch";

static void app_wifi_init()
{
    esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}

// All the default GPIOs are based on ESP32 series DevKitC boards, for other boards, please modify them accordingly.
#ifdef CONFIG_IDF_TARGET_ESP32C2
#define CONTROL_KEY_GPIO      GPIO_NUM_9
#elif CONFIG_IDF_TARGET_ESP32C3
#define CONTROL_KEY_GPIO      GPIO_NUM_9
#elif CONFIG_IDF_TARGET_ESP32
#define CONTROL_KEY_GPIO      GPIO_NUM_0
#elif CONFIG_IDF_TARGET_ESP32S2
#define CONTROL_KEY_GPIO      GPIO_NUM_0
#elif CONFIG_IDF_TARGET_ESP32S3
#define CONTROL_KEY_GPIO      GPIO_NUM_0
#elif CONFIG_IDF_TARGET_ESP32C6
#define CONTROL_KEY_GPIO      GPIO_NUM_9
#endif

static void app_switch_send_press_cb(void *arg, void *usr_data)
{
    uint8_t status = 0;

    ESP_ERROR_CHECK(!(BUTTON_SINGLE_CLICK == iot_button_get_event(arg)));

    ESP_LOGI(TAG, "switch send press");
    /* status = 0: OFF, 1: ON, 2: TOGGLE */
    espnow_storage_get(BULB_STATUS_KEY, &status, sizeof(status));
    status ^= 1;
    espnow_storage_set(BULB_STATUS_KEY, &status, sizeof(status));
    ESP_LOGI(TAG, "key status: %d", status);
    espnow_ctrl_initiator_send(ESPNOW_ATTRIBUTE_KEY_1, ESPNOW_ATTRIBUTE_POWER, status);
}

static void app_switch_bind_press_cb(void *arg, void *usr_data)
{
    ESP_ERROR_CHECK(!(BUTTON_DOUBLE_CLICK == iot_button_get_event(arg)));

    ESP_LOGI(TAG, "switch bind press");
    espnow_ctrl_initiator_bind(ESPNOW_ATTRIBUTE_KEY_1, true);
}

static void app_switch_unbind_press_cb(void *arg, void *usr_data)
{
    ESP_ERROR_CHECK(!(BUTTON_LONG_PRESS_START == iot_button_get_event(arg)));

    ESP_LOGI(TAG, "switch unbind press");
    espnow_ctrl_initiator_bind(ESPNOW_ATTRIBUTE_KEY_1, false);
}

static void app_driver_init(void)
{
    button_config_t button_config = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = CONTROL_KEY_GPIO,
            .active_level = 0,
        },
    };

    button_handle_t button_handle = iot_button_create(&button_config);

    iot_button_register_cb(button_handle, BUTTON_SINGLE_CLICK, app_switch_send_press_cb, NULL);
    iot_button_register_cb(button_handle, BUTTON_DOUBLE_CLICK, app_switch_bind_press_cb, NULL);
    iot_button_register_cb(button_handle, BUTTON_LONG_PRESS_START, app_switch_unbind_press_cb, NULL);
}

void app_main(void)
{
    espnow_storage_init();

    app_wifi_init();
    app_driver_init();

    espnow_config_t espnow_config = ESPNOW_INIT_CONFIG_DEFAULT();
    espnow_config.send_max_timeout = portMAX_DELAY;
    espnow_init(&espnow_config);
    esp_now_set_wake_window(0);
}
