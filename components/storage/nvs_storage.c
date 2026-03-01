/**
 * NVS Storage - Implementation
 */

#include <esp_log.h>
#include <nvs_flash.h>
#include <nvs.h>

static const char *TAG = "NVS_STORAGE";

/**
 * @brief Initialize NVS storage
 */
esp_err_t nvs_storage_init(void)
{
    return nvs_flash_init();
}

/**
 * @brief Store string value
 */
esp_err_t nvs_storage_set_str(const char *namespace, const char *key, const char *value)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;
    
    err = nvs_set_str(handle, key, value);
    if (err == ESP_OK) {
        nvs_commit(handle);
    }
    nvs_close(handle);
    
    return err;
}

/**
 * @brief Get string value
 */
esp_err_t nvs_storage_get_str(const char *namespace, const char *key, char *value, size_t *len)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(namespace, NVS_READONLY, &handle);
    if (err != ESP_OK) return err;
    
    err = nvs_get_str(handle, key, value, len);
    nvs_close(handle);
    
    return err;
}
