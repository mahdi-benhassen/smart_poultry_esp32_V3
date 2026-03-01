/**
 * Storage Manager - Implementation
 */

#include <string.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <nvs.h>
#include "storage_manager.h"

static const char *TAG = "STORAGE_MGR";
static const char *NVS_NAMESPACE = "poultry";

/**
 * @brief Initialize storage
 */
esp_err_t storage_manager_init(void)
{
    ESP_LOGI(TAG, "Storage manager initialized");
    return ESP_OK;
}

/**
 * @brief Get WiFi configuration
 */
esp_err_t storage_get_wifi_config(storage_config_t *config)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    
    if (err != ESP_OK) {
        /* Use empty defaults if NVS not available */
        config->wifi_ssid[0] = '\0';
        config->wifi_password[0] = '\0';
        return ESP_OK;
    }
    
    size_t len = sizeof(config->wifi_ssid);
    nvs_get_str(handle, "wifi_ssid", config->wifi_ssid, &len);
    
    len = sizeof(config->wifi_password);
    nvs_get_str(handle, "wifi_pass", config->wifi_password, &len);
    
    nvs_close(handle);
    
    return ESP_OK;
}

/**
 * @brief Set WiFi configuration
 */
esp_err_t storage_set_wifi_config(const storage_config_t *config)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    
    if (err != ESP_OK) return err;
    
    nvs_set_str(handle, "wifi_ssid", config->wifi_ssid);
    nvs_set_str(handle, "wifi_pass", config->wifi_password);
    nvs_commit(handle);
    nvs_close(handle);
    
    return ESP_OK;
}

/**
 * @brief Log sensor data
 */
esp_err_t storage_log_sensor_data(const void *data)
{
    /* Implementation would write to SPIFFS */
    return ESP_OK;
}
