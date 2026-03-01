/**
 * Cooling Pad Driver
 */

#ifndef COOLING_PAD_H
#define COOLING_PAD_H

#include <stdint.h>
#include <esp_err.h>

/**
 * @brief Initialize cooling pad
 */
esp_err_t cooling_pad_init(void);

/**
 * @brief Set cooling pad on/off
 */
esp_err_t cooling_pad_set(bool on);

/**
 * @brief Get cooling pad state
 */
esp_err_t cooling_pad_get(bool *on);

#endif /* COOLING_PAD_H */
