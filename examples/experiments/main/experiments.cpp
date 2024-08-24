#include <stdio.h>

#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_now.h"
#include "nvs_flash.h"
#include "led_status.hpp"
#include <cstdint>
#include "esp_task_wdt.h" // TODO: remove


static const char* TAG = "experiment";

// If surprised me that esp_wifi_init fails with ESP_ERR_NVS_NOT_INITIALIZED if NVS hasn't been inited.
// But it turns out WIFI_INIT_CONFIG_DEFAULT includes setting the field nvs_enable which is set via sdkconfig and defaults to enabled.
static void app_nvs_init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();                                 
    }
    ESP_ERROR_CHECK( ret ); 
}

static void app_wifi_init() {
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    // Even if you subsequently intend to call esp_wifi_set_storage(WIFI_STORAGE_RAM),
    // esp_wifi_init will check that NVS is initialized unless...
    cfg.nvs_enable = false;

    // TODO: 
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK( esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR) );
    ESP_ERROR_CHECK( esp_wifi_start());

    // esp_wifi_set_channel can only be called after start.
    ESP_ERROR_CHECK( esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE));
}

static constexpr const char* NVS_NAMESPACE = "orion";
static constexpr const char* RESET_RECORD = "reset_record";

void save(const char* key, const void* value, size_t length)
{
    nvs_handle_t handle = 0;

    ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle));
    ESP_ERROR_CHECK(nvs_set_blob(handle, key, value, length));
    ESP_ERROR_CHECK(nvs_commit(handle));

    nvs_close(handle);
}

bool load(const char* key, void* value, size_t length) {
    nvs_handle_t handle = 0;

    // Without write permission, nvs_open will fail rather than create the namespace if it doesn't exist.
    ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle));

    esp_err_t ret = nvs_get_blob(handle, key, value, &length);

    nvs_close(handle);

    switch (ret)
    {
    case ESP_OK: return true;
    case ESP_ERR_NVS_NOT_FOUND: return false;
    default:
        ESP_ERROR_CHECK(ret);
        return false;
    }
}

// Only make `inline` if defining in header - https://stackoverflow.com/a/1759575/245602
void save(const char* key, const auto& value) {
    // TODO: shouldn't &value require a cast - make sure -Wall is on.
    save(key, &value, sizeof(value));
}

void load(const char* key, auto& value) {
    load(key, &value, sizeof(value));
}

struct ResetRecord {
    std::uint32_t total = 0;
    std::uint32_t since_bad = 0;
    esp_reset_reason_t reason = ESP_RST_UNKNOWN;
};

static ResetRecord reset_record;

bool ok_reset_reason(esp_reset_reason_t reason) {
    return
        reason == ESP_RST_POWERON ||
        reason == ESP_RST_SW ||
        reason == ESP_RST_DEEPSLEEP ||
        reason == ESP_RST_SDIO ||
        reason == ESP_RST_USB ||
        reason == ESP_RST_JTAG;
}

// TODO: Guru meditations seem to hang the MCU rather than restarting it - can one change this?
//  Here all you see is the subsequent good RESET button reset rather than the Guru failure.
void app_reset_init() {
    load(RESET_RECORD, reset_record);

    esp_reset_reason_t reason = esp_reset_reason();

    if (ok_reset_reason(reason)) {
        reset_record.since_bad++;
    } else {
        reset_record.since_bad = 0;
        reset_record.reason = reason;
    }

    reset_record.total++;

    save(RESET_RECORD, reset_record);

    ESP_LOGI(TAG, "Total reset count: %lu", reset_record.total);

    if (reset_record.total != reset_record.since_bad) {
        ESP_LOGI(TAG, "Last bad reset reason: %d", reset_record.reason);
        ESP_LOGI(TAG, "Resets since last bad reset: %lu", reset_record.since_bad);
    }
}

extern "C" int app_main(void) {
    // A watch dog timer task gets added just before app_main is called (see app_startup.c:main_task)
    ESP_LOGI(TAG, "esp_task_wdt_status: 0x%04x", esp_task_wdt_status(xTaskGetIdleTaskHandleForCPU(0)));

    app_nvs_init();
    app_wifi_init();
    app_led_init();

    set_led_mode(BLINK_BREATHE_SLOW);

    wifi_country_t wifiCountry;
    uint8_t mac[6];

    // TODO: wrap everything with ESP_ERROR_CHECK.
    ESP_ERROR_CHECK(esp_wifi_get_country(&wifiCountry));
    // "01" means "world safe mode" 20 dBm = 100 mW.
    ESP_LOGI(TAG, "WiFi country code: %.2s, first channel=%d, channel count=%d, max TX power=%d dBm", wifiCountry.cc, wifiCountry.schan, wifiCountry.nchan, wifiCountry.max_tx_power);
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    // MAC address is output when calling esp_wifi_set_mode:
    // "I (695) wifi:mode : sta (64:e8:33:88:a6:20)"
    // ESP_LOGI(TAG, "MAC address: " MACSTR "", MAC2STR(mac));

    app_reset_init();

    // For some reason, I was getting failures in `esp_task_wdt_reset` due to it failing to find the relevant task.
    // But it seems to be present at this point (status == 0x0000) whereas ESP_ERR_NOT_FOUND = 0x0105.
    ESP_LOGI(TAG, "esp_task_wdt_status: 0x%04x", esp_task_wdt_status(xTaskGetIdleTaskHandleForCPU(0)));

    return 0;
}
