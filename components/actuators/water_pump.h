/**
 * Water Pump Driver
 */

#ifndef WATER_PUMP_H
#define WATER_PUMP_H

#include <stdint.h>
#include <esp_err.h>

/**
 * @brief Initialize water pump
 */
esp_err_t water_pump_init(void);

/**
 * @brief Set pump on/off
 */
esp_err_t water_pump_set(bool on);

/**
 * @brief Get pump state
 */
esp_err_t water_pump_get(bool *on);

#endif /* WATER_PUMP_H */
