/**
 * LDR (Light Dependent Resistor) Sensor Driver
 */

#ifndef LDR_H
#define LDR_H

#include <stdint.h>
#include <esp_err.h>
#include <driver/adc.h>
#include "esp_adc/adc_oneshot.h"

/**
 * @brief Initialize LDR sensor
 */
esp_err_t ldr_init(adc_oneshot_unit_handle_t handle, adc1_channel_t adc_channel);

/**
 * @brief Read light level (0-100%)
 */
esp_err_t ldr_read(float *percent);

/**
 * @brief Set calibration
 */
void ldr_set_calibration(float min_adc, float max_adc);

#endif /* LDR_H */
