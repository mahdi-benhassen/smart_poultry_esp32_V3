/**
 * ACS712 Current Sensor Driver
 * For energy monitoring and equipment health detection
 * Critical for poultry farm energy management
 */

#ifndef ACS712_H
#define ACS712_H

#include <stdint.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/adc.h>

/* ACS712 Model Types */
typedef enum {
    ACS712_5A,    /* ±5A range */
    ACS712_20A,   /* ±20A range */
    ACS712_30A    /* ±30A range */
} acs712_model_t;

/**
 * @brief Initialize ACS712 sensor
 */
esp_err_t acs712_init(acs712_model_t model, adc_channel_t adc_channel);

/**
 * @brief Read current in Amperes
 */
esp_err_t acs712_read_current(float *current_amps);

/**
 * @brief Read raw ADC value
 */
esp_err_t acs712_read_raw(int *adc_value);

/**
 * @brief Set zero current offset (for calibration)
 */
esp_err_t acs712_set_offset(int offset);

/**
 * @brief Get zero current offset
 */
int acs712_get_offset(void);

/**
 * @brief Set sensitivity (mV per Ampere)
 */
esp_err_t acs712_set_sensitivity(float sensitivity_mv_per_amp);

/**
 * @brief Get sensitivity
 */
float acs712_get_sensitivity(void);

/**
 * @brief Enable/disable running average filter
 */
esp_err_t acs712_set_filter_enabled(bool enabled);

/**
 * @brief Get filtered current (if filter enabled)
 */
esp_err_t acs712_get_filtered_current(float *current_amps);

/**
 * @brief Get current consumption in Watts (assuming 220V)
 */
esp_err_t acs712_get_power(float *power_watts);

/**
 * @brief Reset cumulative energy counter
 */
esp_err_t acs712_reset_energy(void);

/**
 * @brief Get cumulative energy in Watt-hours
 */
float acs712_get_energy_wh(void);

/**
 * @brief Update energy counter (call periodically)
 */
void acs712_update_energy(void);

#endif /* ACS712_H */
