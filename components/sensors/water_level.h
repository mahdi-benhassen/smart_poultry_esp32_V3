/**
 * Water Level Sensor Driver
 */

#ifndef WATER_LEVEL_H
#define WATER_LEVEL_H

#include <stdint.h>
#include <esp_err.h>
#include "esp_adc/adc_oneshot.h"

/**
 * @brief Initialize water level sensor
 */
esp_err_t water_level_init(adc_oneshot_unit_handle_t handle, adc1_channel_t adc_channel);

/**
 * @brief Read water level (0-100%)
 */
esp_err_t water_level_read(uint8_t *percent);

/**
 * @brief Get water level status
 */
typedef enum {
    WATER_LEVEL_LOW,
    WATER_LEVEL_MEDIUM,
    WATER_LEVEL_HIGH,
    WATER_LEVEL_FULL
} water_level_status_t;

esp_err_t water_level_get_status(water_level_status_t *status);

#endif /* WATER_LEVEL_H */
