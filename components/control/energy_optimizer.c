/**
 * Energy Optimizer - Implementation
 */

#include "energy_optimizer.h"
#include "decision_algorithm.h"

/**
 * @brief Initialize energy optimizer
 */
void energy_optimizer_init(void)
{
}

/**
 * @brief Optimize decision for energy efficiency
 */
void energy_optimizer_optimize(decision_output_t *output)
{
    if (!output) return;
    
    /* Reduce fan speed if temperature is close to optimal */
    if (output->fan1_speed > 50 && output->heater_level == 0) {
        output->fan1_speed = (uint8_t)(output->fan1_speed * 0.8f);
        output->fan2_speed = (uint8_t)(output->fan2_speed * 0.8f);
    }
    
    /* Turn off heater if cooling is on */
    if (output->cooling_pad_on && output->heater_level > 0) {
        output->heater_level = 0;
    }
}
