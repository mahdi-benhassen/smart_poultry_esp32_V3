/**
 * LDR Sensor Driver - Implementation
 */

#include <esp_log.h>
#include <esp_err.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#include "ldr.h"

static const char *TAG = "LDR";

static adc1_channel_t s_adc_channel = ADC_CHANNEL_7;
static bool s_initialized = false;
static float s_min_adc = 0.0f;
static float s_max_adc = 4095.0f;
static esp_adc_cal_characteristics_t *s_adc_chars = NULL;

/**
 * @brief Initialize LDR sensor
 */
esp_err_t ldr_init(adc1_channel_t adc_channel)
{
    s_adc_channel = adc_channel;
    
    ESP_ERROR_CHECK(adc1_config_channel_atten(adc_channel, ADC_ATTEN_DB_11));
    
    s_adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    if (s_adc_chars == NULL) {
        return ESP_ERR_NO_MEM;
    }
    
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, s_adc_chars);
    
    s_initialized = true;
    ESP_LOGI(TAG, "LDR initialized on ADC channel %d", adc_channel);
    
    return ESP_OK;
}

/**
 * @brief Read light level (0-100%)
 */
esp_err_t ldr_read(float *percent)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (percent == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* Read ADC value */
    uint32_t adc_reading = 0;
    for (int i = 0; i < 10; i++) {
        adc_reading += adc1_get_raw(s_adc_channel);
    }
    adc_reading /= 10;
    
    /* Convert to percentage */
    *percent = ((float)(adc_reading - s_min_adc) / (s_max_adc - s_min_adc)) * 100.0f;
    
    /* Clamp */
    if (*percent < 0) *percent = 0;
    if (*percent > 100) *percent = 100;
    
    ESP_LOGD(TAG, "LDR: ADC=%u, Light=%.1f%%", adc_reading, *percent);
    
    return ESP_OK;
}

/**
 * @brief Set calibration
 */
void ldr_set_calibration(float min_adc, float max_adc)
{
    s_min_adc = min_adc;
    s_max_adc = max_adc;
    ESP_LOGI(TAG, "LDR calibration set: min=%.0f, max=%.0f", min_adc, max_adc);
}
