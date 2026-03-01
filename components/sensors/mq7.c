/**
 * MQ-7 Carbon Monoxide Sensor Driver - Implementation
 */

#include <math.h>
#include <esp_log.h>
#include <esp_err.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#include "mq7.h"

static const char *TAG = "MQ7";

/* MQ-7 characteristic curve for CO */
static const float MQ7_Curve[3] = {-0.68, 0.45, 0.37};

static adc1_channel_t s_adc_channel = ADC_CHANNEL_3;
static float s_rl = MQ7_RL_VALUE;
static float s_ro = MQ7_RO_CLEAN_AIR;
static bool s_initialized = false;
static esp_adc_cal_characteristics_t s_adc_chars;

/**
 * @brief Initialize MQ-7 sensor
 */
esp_err_t mq7_init(adc1_channel_t adc_channel)
{
    s_adc_channel = adc_channel;
    
    ESP_ERROR_CHECK(adc1_config_channel_atten(adc_channel, ADC_ATTEN_DB_11));
    
    /* Characterize ADC - use static buffer to avoid memory leak */
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &s_adc_chars);
    
    s_initialized = true;
    ESP_LOGI(TAG, "MQ-7 initialized on ADC channel %d", adc_channel);
    
    return ESP_OK;
}

/**
 * @brief Read CO concentration in PPM
 */
esp_err_t mq7_read(float *ppm)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (ppm == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* Read ADC value */
    uint32_t adc_reading = 0;
    for (int i = 0; i < 10; i++) {
        adc_reading += adc1_get_raw(s_adc_channel);
    }
    adc_reading /= 10;
    
    /* Convert to voltage */
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &s_adc_chars);
    float v_voltage = voltage / 1000.0f;
    
    /* Division by zero protection */
    if (v_voltage < 0.01f) {
        v_voltage = 0.01f;
    }
    
    /* Calculate Rs */
    float rs = ((5.0f - v_voltage) / v_voltage) * s_rl;
    
    /* Calculate ratio and PPM */
    float ratio = rs / s_ro;
    *ppm = MQ7_Curve[0] * pow(ratio, 2) + MQ7_Curve[1] * ratio + MQ7_Curve[2];
    
    /* Clamp to reasonable range */
    if (*ppm < 0) *ppm = 0;
    if (*ppm > 500) *ppm = 500;
    
    ESP_LOGD(TAG, "MQ-7: PPM=%.1f", *ppm);
    
    return ESP_OK;
}

/**
 * @brief Calibrate sensor
 */
esp_err_t mq7_calibrate(float ro)
{
    if (ro <= 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    s_ro = ro;
    ESP_LOGI(TAG, "MQ-7 calibrated: Ro=%.2f", s_ro);
    
    return ESP_OK;
}

/**
 * @brief Get Ro value
 */
float mq7_get_ro(void)
{
    return s_ro;
}
