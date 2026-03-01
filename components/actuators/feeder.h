/**
 * Automatic Feeder Driver
 */

#ifndef FEEDER_H
#define FEEDER_H

#include <stdint.h>
#include <esp_err.h>

/**
 * @brief Initialize feeder
 */
esp_err_t feeder_init(void);

/**
 * @brief Activate feeder (dispense food)
 */
esp_err_t feeder_activate(void);

/**
 * @brief Get feeder state
 */
esp_err_t feeder_get_state(bool *active);

#endif /* FEEDER_H */
