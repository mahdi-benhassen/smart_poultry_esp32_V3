#include <esp_log.h>
#include <esp_err.h>
#include "esp_adc/adc_oneshot.h"

#include "sound_sensor.h"

static const char *TAG = "SOUND_SENSOR";

static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_channel_t s_adc_channel = ADC_CHANNEL_5;
static bool s_initialized = false;
static float s_average = 0.0f;
static uint32_t s_sample_count = 0;

/**
 * @brief Initialize sound sensor
 */
esp_err_t sound_sensor_init(adc_oneshot_unit_handle_t handle, adc_channel_t adc_channel)
{
    s_adc_handle = handle;
    s_adc_channel = adc_channel;
    
    s_initialized = true;
    s_average = 0.0f;
    s_sample_count = 0;
    
    ESP_LOGI(TAG, "Sound sensor initialized");
    
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
    int adc_raw = 0;
    int adc_reading = 0;
    for (int i = 0; i < 10; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(s_adc_handle, s_adc_channel, &adc_raw));
        adc_reading += adc_raw;
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
