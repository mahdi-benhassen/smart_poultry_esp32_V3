/**
 * Sound Sensor Driver - Implementation
 */

#include <esp_log.h>
#include <esp_err.h>
#include <driver/adc.h>

#include "sound_sensor.h"

static const char *TAG = "SOUND_SENSOR";

static adc1_channel_t s_adc_channel = ADC_CHANNEL_5;
static bool s_initialized = false;
static float s_average = 0.0f;
static uint32_t s_sample_count = 0;

/**
 * @brief Initialize sound sensor
 */
esp_err_t sound_sensor_init(adc1_channel_t adc_channel)
{
    s_adc_channel = adc_channel;
    
    ESP_ERROR_CHECK(adc1_config_channel_atten(adc_channel, ADC_ATTEN_DB_11));
    
    s_initialized = true;
    s_average = 0.0f;
    s_sample_count = 0;
    
    ESP_LOGI(TAG, "Sound sensor initialized on ADC channel %d", adc_channel);
    
    return ESP_OK;
}

/**
 * @brief Read sound level (0-100%)
 */
esp_err_t sound_sensor_read(float *level)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (level == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* Read multiple samples for more stable reading */
    uint32_t adc_reading = 0;
    for (int i = 0; i < 10; i++) {
        adc_reading += adc1_get_raw(s_adc_channel);
    }
    adc_reading /= 10;
    
    /* Convert to percentage (0-100) */
    *level = ((float)adc_reading / 4095.0f) * 100.0f;
    
    /* Update running average */
    s_sample_count++;
    if (s_sample_count == 1) {
        s_average = *level;
    } else {
        s_average = s_average + (*level - s_average) / s_sample_count;
    }
    
    ESP_LOGD(TAG, "Sound level: %.1f%%", *level);
    
    return ESP_OK;
}

/**
 * @brief Get average sound level
 */
esp_err_t sound_sensor_get_average(float *level)
{
    if (level == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *level = s_average;
    return ESP_OK;
}
