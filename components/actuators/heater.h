/**
 * Heater Driver
 */

#ifndef HEATER_H
#define HEATER_H

#include <stdint.h>
#include <esp_err.h>

/**
 * @brief Initialize heater
 */
esp_err_t heater_init(void);

/**
 * @brief Set heater level (0-100%)
 */
esp_err_t heater_set_level(uint8_t level);

/**
 * @brief Get heater level
 */
esp_err_t heater_get_level(uint8_t *level);

#endif /* HEATER_H */
