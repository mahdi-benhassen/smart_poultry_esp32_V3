/**
 * Exhaust Fan Driver
 */

#ifndef EXHAUST_FAN_H
#define EXHAUST_FAN_H

#include <stdint.h>
#include <esp_err.h>

/* Fan configurations */
#define FAN_COUNT        2

/**
 * @brief Initialize exhaust fans
 */
esp_err_t exhaust_fan_init(void);

/**
 * @brief Set fan speed
 * @param fan_id 0 or 1
 * @param speed 0-100%
 */
esp_err_t exhaust_fan_set_speed(uint8_t fan_id, uint8_t speed);

/**
 * @brief Get fan speed
 */
esp_err_t exhaust_fan_get_speed(uint8_t fan_id, uint8_t *speed);

/**
 * @brief Set all fans to specific speed
 */
esp_err_t exhaust_fan_set_all(uint8_t speed);

#endif /* EXHAUST_FAN_H */
