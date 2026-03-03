/**
 * Emergency Protocol - Header File
 * Tunisian Mediterranean Climate Poultry Standards
 */

#ifndef EMERGENCY_PROTOCOL_H
#define EMERGENCY_PROTOCOL_H

#include <stdbool.h>
#include "sensor_manager.h"

/**
 * @brief Initialize emergency protocol
 */
void emergency_protocol_init(void);

/**
 * @brief Clear emergency state and re-enable actuators
 */
void emergency_protocol_clear(void);

/**
 * @brief Get current emergency status
 * @return true if emergency is active
 */
bool emergency_protocol_is_active(void);

/**
 * @brief Check if emergency condition exists
 */
bool emergency_protocol_check(const sensor_data_t *data);

#endif /* EMERGENCY_PROTOCOL_H */
