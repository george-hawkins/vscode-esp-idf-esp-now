/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_log.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "esp_wifi.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
#include "esp_mac.h"
#endif

#include "espnow_utils.h"

#include "espnow.h"
#include "espnow_ctrl.h"

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#include "driver/rmt.h"
#endif

#include "driver/gpio.h"

// All the default GPIOs are based on ESP32 series DevKitC boards, for other boards, please modify them accordingly.
#ifdef CONFIG_IDF_TARGET_ESP32C2
#define CONTROL_KEY_GPIO      GPIO_NUM_9
#define LED_RED_GPIO          GPIO_NUM_0
#define LED_GREEN_GPIO        GPIO_NUM_1
#define LED_BLUE_GPIO         GPIO_NUM_8
#elif CONFIG_IDF_TARGET_ESP32C3
#define CONTROL_KEY_GPIO      GPIO_NUM_9
#define LED_STRIP_GPIO        GPIO_NUM_8
#elif CONFIG_IDF_TARGET_ESP32
#define CONTROL_KEY_GPIO      GPIO_NUM_0
// There is not LED module in ESP32 DevKitC board, so you need to connect one by yourself.
#define LED_STRIP_GPIO        GPIO_NUM_18
#elif CONFIG_IDF_TARGET_ESP32S2
#define CONTROL_KEY_GPIO      GPIO_NUM_0
#define LED_STRIP_GPIO        GPIO_NUM_18
#elif CONFIG_IDF_TARGET_ESP32S3
#define CONTROL_KEY_GPIO      GPIO_NUM_0
// For old version board, the number is 48.
#define LED_STRIP_GPIO        GPIO_NUM_38
#elif CONFIG_IDF_TARGET_ESP32C6
#define CONTROL_KEY_GPIO      GPIO_NUM_9
#define LED_STRIP_GPIO        GPIO_NUM_8
#endif

#define UNBIND_TOTAL_COUNT               (5)

#define BULB_STATUS_KEY                  "bulb_key"

static const char *TAG = "app_bulb";

static uint32_t s_bulb_status = 0;

static void app_set_bulb_status(void)
{
    espnow_storage_set(BULB_STATUS_KEY, &s_bulb_status, sizeof(s_bulb_status));
}

static void app_get_bulb_status(void)
{
    espnow_storage_get(BULB_STATUS_KEY, &s_bulb_status, sizeof(s_bulb_status));
}

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

static void app_led_init(void)
{
    gpio_reset_pin(LED_STRIP_GPIO);
    gpio_set_direction(LED_STRIP_GPIO, GPIO_MODE_OUTPUT);
}

void app_led_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
    uint32_t level = red == 0 && green == 0 && blue == 0 ? 0 : 1;

    gpio_set_level(LED_STRIP_GPIO, level);
}

static void app_driver_init(void)
{
    app_led_init();
}

static void app_bulb_ctrl_data_cb(espnow_attribute_t initiator_attribute,
                              espnow_attribute_t responder_attribute,
                              uint32_t status)
{
    ESP_LOGI(TAG, "app_bulb_ctrl_data_cb, initiator_attribute: %d, responder_attribute: %d, value: %" PRIu32 "",
             initiator_attribute, responder_attribute, status);
    /* status = 0: OFF, 1: ON, 2: TOGGLE */
    if (status != s_bulb_status) {
        s_bulb_status ^= 1;
        app_set_bulb_status();
        if (s_bulb_status) {
            app_led_set_color(255, 255, 255);
        } else {
            app_led_set_color(0, 0, 0);
        }
    }
}

static void app_bulb_init(void)
{
    ESP_ERROR_CHECK(espnow_ctrl_responder_bind(30 * 60 * 1000, -55, NULL));
    espnow_ctrl_responder_data(app_bulb_ctrl_data_cb);
}

static void app_espnow_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    if (base != ESP_EVENT_ESPNOW) {
        return;
    }

    switch (id) {
    case ESP_EVENT_ESPNOW_CTRL_BIND: {
        espnow_ctrl_bind_info_t *info = (espnow_ctrl_bind_info_t *)event_data;
        ESP_LOGI(TAG, "bind, uuid: " MACSTR ", initiator_type: %d", MAC2STR(info->mac), info->initiator_attribute);

        app_led_set_color(0, 255, 0);
        break;
    }

    case ESP_EVENT_ESPNOW_CTRL_UNBIND: {
        espnow_ctrl_bind_info_t *info = (espnow_ctrl_bind_info_t *)event_data;
        ESP_LOGI(TAG, "unbind, uuid: " MACSTR ", initiator_type: %d", MAC2STR(info->mac), info->initiator_attribute);

        app_led_set_color(255, 0, 0);
        break;
    }

    default:
        break;
    }
}

static esp_err_t unbind_restore_init(void)
{
    if (espnow_reboot_unbroken_count() >= UNBIND_TOTAL_COUNT) {
        ESP_LOGI(TAG, "unbind restore");
        espnow_storage_erase("bindlist");
        app_led_set_color(255, 0, 0);
        return espnow_reboot(CONFIG_ESPNOW_REBOOT_UNBROKEN_INTERVAL_TIMEOUT);
    }

    return ESP_OK;
}

void app_main(void)
{
    espnow_storage_init();
    app_driver_init();

    unbind_restore_init();

    app_get_bulb_status();
    if (s_bulb_status) {
        app_led_set_color(255, 255, 255);
    } else {
        app_led_set_color(0, 0, 0);
    }

    app_wifi_init();

    espnow_config_t espnow_config = ESPNOW_INIT_CONFIG_DEFAULT();
    espnow_init(&espnow_config);

    esp_event_handler_register(ESP_EVENT_ESPNOW, ESP_EVENT_ANY_ID, app_espnow_event_handler, NULL);

    app_bulb_init();
}
