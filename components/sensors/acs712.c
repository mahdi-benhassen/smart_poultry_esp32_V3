/**
 * ACS712 Current Sensor Driver - Implementation
 * For energy monitoring and equipment health detection
 * Critical for poultry farm energy management
 */

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "acs712.h"

static const char *TAG = "ACS712";

/* ADC Configuration */
#define ACS712_ADC_WIDTH     ADC_BITWIDTH_12
#define ACS712_ADC_ATTEN     ADC_ATTEN_DB_11

/* Voltage reference (mV) */
#define ACS712_VREF_MV       3300.0f

/* Default sensitivities (mV/A) */
#define ACS712_5A_SENSITIVITY    185.0f   /* 185mV/A for ±5A */
#define ACS712_20A_SENSITIVITY   100.0f   /* 100mV/A for ±20A */
#define ACS712_30A_SENSITIVITY   66.0f    /* 66mV/A for ±30A */

/* Zero current voltage (half of VCC for 5V supply) */
#define ACS712_ZERO_VOLTAGE      2500.0f  /* 2.5V for 5V supply */

/* Global state */
static adc_channel_t s_adc_channel = ADC_CHANNEL_0;
static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static bool s_initialized = false;
static acs712_model_t s_model = ACS712_20A;
static float s_sensitivity = ACS712_20A_SENSITIVITY;
static int s_offset = 0;
static bool s_filter_enabled = true;
static int s_filtered_value = 0;

/* Energy monitoring */
static float s_cumulative_energy_wh = 0.0f;
static float s_last_current = 0.0f;
static uint32_t s_last_update_time = 0;

/**
 * @brief Initialize ACS712 sensor
 */
esp_err_t acs712_init(acs712_model_t model, adc_channel_t adc_channel)
{
    s_model = model;
    s_adc_channel = adc_channel;
    
    /* Set sensitivity based on model */
    switch (model) {
        case ACS712_5A:
            s_sensitivity = ACS712_5A_SENSITIVITY;
            break;
        case ACS712_20A:
            s_sensitivity = ACS712_20A_SENSITIVITY;
            break;
        case ACS712_30A:
            s_sensitivity = ACS712_30A_SENSITIVITY;
            break;
    }
    
    /* Initialize ADC Oneshot Unit */
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    
    esp_err_t ret = adc_oneshot_new_unit(&init_config, &s_adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create ADC oneshot unit");
        return ret;
    }
    
    /* Configure ADC channel */
    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ACS712_ADC_ATTEN,
        .bitwidth = ACS712_ADC_WIDTH,
    };
    
    ret = adc_oneshot_config_channel(s_adc_handle, adc_channel, &chan_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ADC channel");
        return ret;
    }
    
    s_initialized = true;
    s_last_update_time = esp_log_timestamp();
    
    ESP_LOGI(TAG, "ACS712 initialized: model=%d, ADC channel=%d, sensitivity=%.1fmV/A", 
             model, adc_channel, s_sensitivity);
    
    return ESP_OK;
}

/**
 * @brief Read raw ADC value
 */
esp_err_t acs712_read_raw(int *adc_value)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (adc_value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* Read ADC value */
    int raw;
    esp_err_t ret = adc_oneshot_read(s_adc_handle, s_adc_channel, &raw);
    if (ret != ESP_OK) {
        return ret;
    }
    
    /* Apply offset */
    *adc_value = raw - s_offset;
    
    ESP_LOGD(TAG, "Raw ADC: %d (offset: %d)", raw, s_offset);
    
    return ESP_OK;
}

/**
 * @brief Read current in Amperes
 */
esp_err_t acs712_read_current(float *current_amps)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (current_amps == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int adc_value;
    esp_err_t ret = acs712_read_raw(&adc_value);
    if (ret != ESP_OK) {
        return ret;
    }
    
    /* Convert ADC value to voltage (mV) */
    float voltage_mv = (float)adc_value * ACS712_VREF_MV / 4096.0f;
    
    /* Calculate current: I = (V - Vzero) / Sensitivity */
    float current = (voltage_mv - ACS712_ZERO_VOLTAGE) / s_sensitivity;
    
    /* Apply simple moving average filter */
    if (s_filter_enabled) {
        s_filtered_value = (s_filtered_value * 7 + (int)(current * 100)) / 8;
        current = (float)s_filtered_value / 100.0f;
    }
    
    /* Handle negative values (bidirectional current) */
    if (current < 0 && current > -0.1f) {
        current = 0.0f;  /* Ignore small negative values (noise) */
    }
    
    *current_amps = current;
    s_last_current = current;
    
    ESP_LOGD(TAG, "Current: %.2fA (voltage: %.1fmV)", current, voltage_mv);
    
    return ESP_OK;
}

/**
 * @brief Set zero current offset
 */
esp_err_t acs712_set_offset(int offset)
{
    s_offset = offset;
    ESP_LOGI(TAG, "Offset set to: %d", offset);
    return ESP_OK;
}

/**
 * @brief Get zero current offset
 */
int acs712_get_offset(void)
{
    return s_offset;
}

/**
 * @brief Set sensitivity
 */
esp_err_t acs712_set_sensitivity(float sensitivity_mv_per_amp)
{
    s_sensitivity = sensitivity_mv_per_amp;
    ESP_LOGI(TAG, "Sensitivity set to: %.1fmV/A", sensitivity_mv_per_amp);
    return ESP_OK;
}

/**
 * @brief Get sensitivity
 */
float acs712_get_sensitivity(void)
{
    return s_sensitivity;
}

/**
 * @brief Enable/disable running average filter
 */
esp_err_t acs712_set_filter_enabled(bool enabled)
{
    s_filter_enabled = enabled;
    ESP_LOGI(TAG, "Filter %s", enabled ? "enabled" : "disabled");
    return ESP_OK;
}

/**
 * @brief Get filtered current
 */
esp_err_t acs712_get_filtered_current(float *current_amps)
{
    if (!s_filter_enabled) {
        return acs712_read_current(current_amps);
    }
    
    if (current_amps == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *current_amps = (float)s_filtered_value / 100.0f;
    return ESP_OK;
}

/**
 * @brief Get power consumption in Watts
 */
esp_err_t acs712_get_power(float *power_watts)
{
    float current;
    esp_err_t ret = acs712_read_current(&current);
    
    if (ret == ESP_OK && power_watts != NULL) {
        /* Assuming 220V supply */
        *power_watts = current * 220.0f;
    }
    
    return ret;
}

/**
 * @brief Update energy counter
 */
void acs712_update_energy(void)
{
    if (!s_initialized) {
        return;
    }
    
    uint32_t now = esp_log_timestamp();
    uint32_t elapsed_ms = now - s_last_update_time;
    
    if (elapsed_ms > 0) {
        /* Energy in Wh = Power (W) * Time (h) */
        float elapsed_hours = (float)elapsed_ms / 3600000.0f;
        float power_watts = s_last_current * 220.0f;
        
        s_cumulative_energy_wh += power_watts * elapsed_hours;
        s_last_update_time = now;
    }
}

/**
 * @brief Reset cumulative energy counter
 */
esp_err_t acs712_reset_energy(void)
{
    s_cumulative_energy_wh = 0.0f;
    s_last_update_time = esp_log_timestamp();
    ESP_LOGI(TAG, "Energy counter reset");
    return ESP_OK;
}

/**
 * @brief Get cumulative energy in Watt-hours
 */
float acs712_get_energy_wh(void)
{
    /* Update before returning */
    acs712_update_energy();
    return s_cumulative_energy_wh;
}
