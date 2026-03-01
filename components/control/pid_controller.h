/**
 * PID Controller - Header File
 */

#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <stdint.h>

/**
 * @brief Initialize PID controller
 */
void pid_controller_init(void);

/**
 * @brief Set PID parameters
 */
void pid_controller_set_params(float kp, float ki, float kd);

/**
 * @brief Compute PID output
 * @param setpoint Target value
 * @param process_value Current value
 * @return Control output (0-100)
 */
float pid_controller_compute(float setpoint, float process_value);

/**
 * @brief Reset PID controller
 */
void pid_controller_reset(void);

#endif /* PID_CONTROLLER_H */
