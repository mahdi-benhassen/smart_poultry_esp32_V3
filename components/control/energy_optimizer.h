/**
 * Energy Optimizer - Header File
 */

#ifndef ENERGY_OPTIMIZER_H
#define ENERGY_OPTIMIZER_H

#include "decision_algorithm.h"

/**
 * @brief Initialize energy optimizer
 */
void energy_optimizer_init(void);

/**
 * @brief Optimize decision for energy efficiency
 */
void energy_optimizer_optimize(decision_output_t *output);

#endif /* ENERGY_OPTIMIZER_H */
