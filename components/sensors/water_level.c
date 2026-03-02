#include <esp_log.h>
#include <esp_err.h>
#include "esp_adc/adc_oneshot.h"

#include "water_level.h"

static const char *TAG = "WATER_LEVEL";

static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_channel_t s_adc_channel = ADC_CHANNEL_4;
static bool s_initialized = false;
static uint8_t s_last_level = 0;

/* Thresholds */
#define WATER_LEVEL_LOW_THRESH      25
#define WATER_LEVEL_MEDIUM_THRESH    50
#define WATER_LEVEL_HIGH_THRESH     75
#define WATER_LEVEL_FULL_THRESH     95

/**
 * @brief Initialize water level sensor
 */
esp_err_t water_level_init(adc_oneshot_unit_handle_t handle, adc_channel_t adc_channel)
{
    s_adc_handle = handle;
    s_adc_channel = adc_channel;
    
    s_initialized = true;
    ESP_LOGI(TAG, "Water level sensor initialized");
    
    return ESP_OK;
}

/**
 * @brief Read water level (0-100%)
 */
esp_err_t water_level_read(uint8_t *percent)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (percent == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* Read ADC value */
    int adc_raw = 0;
    int adc_reading = 0;
    for (int i = 0; i < 5; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(s_adc_handle, s_adc_channel, &adc_raw));
        adc_reading += adc_raw;
    }
    adc_reading /= 5;
    
    /* Convert to percentage (0-100) */
    /* For analog water level sensor: higher ADC = more water */
    *percent = (uint8_t)((adc_reading * 100) / 4095);
    s_last_level = *percent;
    
    ESP_LOGD(TAG, "Water level: %d%%", *percent);
    
    return ESP_OK;
}

/**
 * @brief Get water level status
 */
esp_err_t water_level_get_status(water_level_status_t *status)
{
    if (status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_last_level < WATER_LEVEL_LOW_THRESH) {
        *status = WATER_LEVEL_LOW;
    } else if (s_last_level < WATER_LEVEL_MEDIUM_THRESH) {
        *status = WATER_LEVEL_MEDIUM;
    } else if (s_last_level < WATER_LEVEL_HIGH_THRESH) {
        *status = WATER_LEVEL_HIGH;
    } else {
        *status = WATER_LEVEL_FULL;
    }
    
    return ESP_OK;
}
