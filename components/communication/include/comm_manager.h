/**
 * Communication Manager - Header File
 */

#ifndef COMM_MANAGER_H
#define COMM_MANAGER_H

#include <esp_err.h>

/**
 * @brief Initialize communication
 */
esp_err_t comm_manager_init(void);

/**
 * @brief Initialize MQTT client
 */
esp_err_t comm_manager_init_mqtt(void);

/**
 * @brief Publish sensor data
 */
esp_err_t comm_manager_publish_sensors(const void *data);

/**
 * @brief Publish actuator state
 */
esp_err_t comm_manager_publish_actuators(const void *data);

/**
 * @brief Check for OTA updates
 */
esp_err_t comm_manager_check_ota(void);

#endif /* COMM_MANAGER_H */
