/**
 * WiFi Manager - Implementation
 */

#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

static const char *TAG = "WIFI_MGR";

/**
 * @brief Initialize WiFi
 */
esp_err_t wifi_manager_init(void)
{
    ESP_LOGI(TAG, "WiFi Manager initialized");
    return ESP_OK;
}

/**
 * @brief Connect to WiFi
 */
esp_err_t wifi_manager_connect(const char *ssid, const char *password)
{
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "WiFi connecting to %s...", ssid);
    
    return ESP_OK;
}

/**
 * @brief Disconnect from WiFi
 */
esp_err_t wifi_manager_disconnect(void)
{
    return esp_wifi_disconnect();
}

/**
 * @brief Get WiFi status
 */
bool wifi_manager_is_connected(void)
{
    wifi_ap_record_t ap_info;
    return (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK);
}
