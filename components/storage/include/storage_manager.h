/**
 * Storage Manager - Header File
 */

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <esp_err.h>

typedef struct {
    char wifi_ssid[32];
    char wifi_password[64];
} storage_config_t;

/**
 * @brief Initialize storage
 */
esp_err_t storage_manager_init(void);

/**
 * @brief Get WiFi configuration
 */
esp_err_t storage_get_wifi_config(storage_config_t *config);

/**
 * @brief Set WiFi configuration
 */
esp_err_t storage_set_wifi_config(const storage_config_t *config);

/**
 * @brief Log sensor data
 */
esp_err_t storage_log_sensor_data(const void *data);

#endif /* STORAGE_MANAGER_H */
