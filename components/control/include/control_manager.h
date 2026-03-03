#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

#include <esp_err.h>
#include "sensor_manager.h"

/**
 * @brief Initialize control system
 */
esp_err_t control_manager_init(void);

/**
 * @brief Update control loop
 */
esp_err_t control_manager_update(void);

/**
 * @brief Update sensor data
 */
esp_err_t control_manager_update_sensors(const sensor_data_t *data);

/**
 * @brief Get current sensor data
 */
sensor_data_t* control_manager_get_sensor_data(void);

/**
 * @brief Update configuration
 */
esp_err_t control_manager_set_config(const void *config);

/**
 * @brief Get configuration
 */
esp_err_t control_manager_get_config(void *config);

#endif /* CONTROL_MANAGER_H */