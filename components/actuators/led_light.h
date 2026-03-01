/**
 * LED Light Driver
 */

#ifndef LED_LIGHT_H
#define LED_LIGHT_H

#include <stdint.h>
#include <esp_err.h>

/**
 * @brief Initialize LED light
 */
esp_err_t led_light_init(void);

/**
 * @brief Set light level (0-100%)
 */
esp_err_t led_light_set_level(uint8_t level);

/**
 * @brief Get light level
 */
esp_err_t led_light_get_level(uint8_t *level);

#endif /* LED_LIGHT_H */
