#include <esp_log.h>
#include <esp_err.h>
#include "esp_adc/adc_oneshot.h"

#include "ldr.h"

static const char *TAG = "LDR";

static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_channel_t s_adc_channel = ADC_CHANNEL_7;
static bool s_initialized = false;
static float s_min_adc = 0.0f;
static float s_max_adc = 4095.0f;

/**
 * @brief Initialize LDR sensor
 */
esp_err_t ldr_init(adc_oneshot_unit_handle_t handle, adc1_channel_t adc_channel)
{
    s_adc_handle = handle;
    s_adc_channel = (adc_channel_t)adc_channel;
    
    s_initialized = true;
    ESP_LOGI(TAG, "LDR initialized");
    
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
    int adc_raw = 0;
    int adc_reading = 0;
    for (int i = 0; i < 10; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(s_adc_handle, s_adc_channel, &adc_raw));
        adc_reading += adc_raw;
    }
    adc_reading /= 10;
    
    /* Convert to percentage */
    *percent = ((float)(adc_reading - s_min_adc) / (s_max_adc - s_min_adc)) * 100.0f;
    
    /* Clamp */
    if (*percent < 0) *percent = 0;
    if (*percent > 100) *percent = 100;
    
    ESP_LOGD(TAG, "LDR: ADC=%d, Light=%.1f%%", adc_reading, *percent);
    
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
