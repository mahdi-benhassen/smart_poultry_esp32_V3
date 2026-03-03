/**
 * Storage Manager - Implementation
 * Modular architecture - storage options can be enabled/disabled via Kconfig
 */

#include <string.h>
#include <esp_log.h>
#include "storage_manager.h"

#ifdef CONFIG_ENABLE_STORAGE_NVS
#include <nvs_flash.h>
#include <nvs.h>
#endif

static const char *TAG = "STORAGE_MGR";
static const char *NVS_NAMESPACE = "poultry";

/**
 * @brief Initialize storage
 */
esp_err_t storage_manager_init(void)
{
    ESP_LOGI(TAG, "Storage manager initialized (Modular)");
    return ESP_OK;
}

/**
 * @brief Get WiFi configuration
 */
esp_err_t storage_get_wifi_config(storage_config_t *config)
{
#ifdef CONFIG_ENABLE_STORAGE_NVS
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    
    if (err != ESP_OK) {
        config->wifi_ssid[0] = '\0';
        config->wifi_password[0] = '\0';
        return ESP_OK;
    }
    
    size_t len = sizeof(config->wifi_ssid);
    nvs_get_str(handle, "wifi_ssid", config->wifi_ssid, &len);
    
    len = sizeof(config->wifi_password);
    nvs_get_str(handle, "wifi_pass", config->wifi_password, &len);
    
    nvs_close(handle);
#endif
    return ESP_OK;
}

/**
 * @brief Set WiFi configuration
 */
esp_err_t storage_set_wifi_config(const storage_config_t *config)
{
#ifdef CONFIG_ENABLE_STORAGE_NVS
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    
    if (err != ESP_OK) return err;
    
    nvs_set_str(handle, "wifi_ssid", config->wifi_ssid);
    nvs_set_str(handle, "wifi_pass", config->wifi_password);
    nvs_commit(handle);
    nvs_close(handle);
#endif
    return ESP_OK;
}

/**
 * @brief Log sensor data
 */
esp_err_t storage_log_sensor_data(const void *data)
{
#ifdef CONFIG_ENABLE_STORAGE_SPIFFS
    /* Implementation would write to SPIFFS */
#endif
    return ESP_OK;
}
