/**
 * PID Controller - Implementation
 */

#include <math.h>
#include <stdbool.h>
#include "pid_controller.h"

static struct {
    float kp;
    float ki;
    float kd;
    float setpoint;
    float integral;
    float prev_error;
    float output;
    bool initialized;
} s_pid = {
    .kp = 2.0f,
    .ki = 0.5f,
    .kd = 0.1f,
    .setpoint = 25.0f,
    .integral = 0.0f,
    .prev_error = 0.0f,
    .output = 0.0f,
    .initialized = false
};

#define PID_OUTPUT_MIN   0.0f
#define PID_OUTPUT_MAX   100.0f
#define PID_INTEGRAL_MAX 1000.0f

/**
 * @brief Initialize PID controller
 */
void pid_controller_init(void)
{
    s_pid.integral = 0.0f;
    s_pid.prev_error = 0.0f;
    s_pid.output = 0.0f;
    s_pid.initialized = true;
}

/**
 * @brief Set PID parameters
 */
void pid_controller_set_params(float kp, float ki, float kd)
{
    s_pid.kp = kp;
    s_pid.ki = ki;
    s_pid.kd = kd;
}

/**
 * @brief Compute PID output
 */
float pid_controller_compute(float setpoint, float process_value)
{
    if (!s_pid.initialized) {
        pid_controller_init();
    }
    
    /* Calculate error */
    float error = setpoint - process_value;
    
    /* Proportional term */
    float p_term = s_pid.kp * error;
    
    /* Integral term with anti-windup */
    s_pid.integral += error;
    if (s_pid.integral > PID_INTEGRAL_MAX) s_pid.integral = PID_INTEGRAL_MAX;
    if (s_pid.integral < -PID_INTEGRAL_MAX) s_pid.integral = -PID_INTEGRAL_MAX;
    float i_term = s_pid.ki * s_pid.integral;
    
    /* Derivative term */
    float d_term = s_pid.kd * (error - s_pid.prev_error);
    s_pid.prev_error = error;
    
    /* Calculate output */
    s_pid.output = p_term + i_term + d_term;
    
    /* Clamp output */
    if (s_pid.output > PID_OUTPUT_MAX) s_pid.output = PID_OUTPUT_MAX;
    if (s_pid.output < PID_OUTPUT_MIN) s_pid.output = PID_OUTPUT_MIN;
    
    return s_pid.output;
}

/**
 * @brief Reset PID controller
 */
void pid_controller_reset(void)
{
    s_pid.integral = 0.0f;
    s_pid.prev_error = 0.0f;
    s_pid.output = 0.0f;
}
