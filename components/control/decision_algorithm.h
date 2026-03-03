/**
 * Decision Algorithm - Header File
 * Tunisian Mediterranean Climate Poultry Standards
 */

#ifndef DECISION_ALGORITHM_H
#define DECISION_ALGORITHM_H

#include <stdint.h>
#include <stdbool.h>
#include "../../main/include/main.h"

/* Decision output */
typedef struct decision_output {
    uint8_t fan1_speed;
    uint8_t fan2_speed;
    bool cooling_pad_on;
    uint8_t heater_level;
    uint8_t light_level;
    bool feeder_activate;
    bool pump_on;
} decision_output_t;

/**
 * @brief Initialize decision algorithm
 */
void decision_algorithm_init(void);

/**
 * @brief Set flock age in days
 */
void decision_algorithm_set_flock_age(uint16_t age_days);

/**
 * @brief Set flock type (broiler or layer)
 */
void decision_algorithm_set_flock_type(flock_type_t type);

/**
 * @brief Evaluate and make decision
 */
void decision_algorithm_evaluate(const sensor_data_t *data, decision_output_t *output);

/**
 * @brief Check water consumption alert
 * @param current_consumption Current water consumption
 * @return true if consumption dropped >10%
 */
bool decision_algorithm_check_water_alert(float current_consumption);

#endif /* DECISION_ALGORITHM_H */
