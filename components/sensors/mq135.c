/**
 * MQ-135 Gas Sensor Driver - Implementation
 */

#include <math.h>
#include <esp_log.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "mq135.h"

static const char *TAG = "MQ135";

/* MQ-135 characteristic curve parameters */
static const float MQ135_Curve[3] = {3.8912, -0.4704, 0.4176};  /* For NH3 */
/* static const float MQ135_Curve_CO2[3] = {4.4887, -0.6468, 0.6328}; */ /* For CO2 - Unused */

/* Global state */
static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_cali_handle_t s_cali_handle = NULL;
static adc_channel_t s_adc_channel = ADC_CHANNEL_0;
static float s_rl = MQ135_RL_VALUE;
static float s_ro = MQ135_RO_CLEAN_AIR;
static bool s_initialized = false;

/**
 * @brief Initialize MQ-135 sensor
 */
esp_err_t mq135_init(adc_oneshot_unit_handle_t handle, adc_channel_t adc_channel)
{
    s_adc_handle = handle;
    s_adc_channel = adc_channel;
    
    /* Calibration handle */
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    
    esp_err_t ret = adc_cali_create_scheme_line_fitting(&cali_config, &s_cali_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "ADC calibration not supported: %s", esp_err_to_name(ret));
    }
    
    s_initialized = true;
    ESP_LOGI(TAG, "MQ-135 sensor initialized");
    
    return ESP_OK;
}

/**
 * @brief Read gas concentration in PPM
 */
esp_err_t mq135_read(float *ppm)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (ppm == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* Read ADC value */
    int adc_raw = 0;
    int adc_reading = 0;
    for (int i = 0; i < 10; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(s_adc_handle, s_adc_channel, &adc_raw));
        adc_reading += adc_raw;
    }
    adc_reading /= 10;
    
    /* Convert to voltage */
    int voltage_mv = 0;
    if (s_cali_handle) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(s_cali_handle, adc_reading, &voltage_mv));
    } else {
        voltage_mv = (adc_reading * 3300) / 4095;
    }
    
    /* Calculate Rs - add division by zero protection */
    float v_voltage = voltage_mv / 1000.0f;  /* Convert mV to V */
    if (v_voltage < 0.01f) {
        v_voltage = 0.01f;  /* Prevent division by zero */
    }
    float rs = ((5.0f - v_voltage) / v_voltage) * s_rl;
    
    /* Calculate ratio */
    float ratio = rs / s_ro;
    
    /* Calculate PPM using curve */
    *ppm = MQ135_Curve[0] * pow(ratio, 2) + MQ135_Curve[1] * ratio + MQ135_Curve[2];
    
    /* Clamp to reasonable range */
    if (*ppm < 0) *ppm = 0;
    if (*ppm > 1000) *ppm = 1000;
    
    ESP_LOGD(TAG, "MQ-135: ADC=%d, Voltage=%.2fV, Rs=%.2f, Ratio=%.2f, PPM=%.1f",
             adc_reading, v_voltage, rs, ratio, *ppm);
    
    return ESP_OK;
}

/**
 * @brief Calibrate sensor
 */
esp_err_t mq135_calibrate(float ro)
{
    if (ro <= 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* Read in clean air for calibration */
    int adc_raw = 0;
    int adc_reading = 0;
    for (int i = 0; i < 50; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(s_adc_handle, s_adc_channel, &adc_raw));
        adc_reading += adc_raw;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    adc_reading /= 50;
    
    int voltage_mv = 0;
    if (s_cali_handle) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(s_cali_handle, adc_reading, &voltage_mv));
    } else {
        voltage_mv = (adc_reading * 3300) / 4095;
    }
    
    float v_voltage = voltage_mv / 1000.0f;
    float rs = ((5.0f - v_voltage) / v_voltage) * s_rl;
    
    s_ro = rs / 4.4f;  /* Clean air factor for NH3 */
    
    ESP_LOGI(TAG, "MQ-135 calibrated: Ro=%.2f", s_ro);
    
    return ESP_OK;
}

/**
 * @brief Get Ro value
 */
float mq135_get_ro(void)
{
    return s_ro;
}

/**
 * @brief Set RL value
 */
void mq135_set_rl(float rl)
{
    if (rl > 0) {
        s_rl = rl;
    }
}

/**
 * @brief Deinitialize MQ-135 sensor
 */
esp_err_t mq135_deinit(void)
{
    s_initialized = false;
    ESP_LOGI(TAG, "MQ-135 deinitialized");
    return ESP_OK;
}
esp_err_t mq135_heater_enable(bool enable)
{
    /* MQ-135 heater is typically always on when powered */
    /* This function can be used if heater is controlled by MOSFET */
    ESP_LOGI(TAG, "MQ-135 heater %s", enable ? "enabled" : "disabled");
    return ESP_OK;
}
