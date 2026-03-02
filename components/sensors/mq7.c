#include <math.h>
#include <esp_log.h>
#include <esp_err.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "mq7.h"

static const char *TAG = "MQ7";

/* MQ-7 characteristic curve for CO */
static const float MQ7_Curve[3] = {-0.68, 0.45, 0.37};

static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_cali_handle_t s_cali_handle = NULL;
static adc_channel_t s_adc_channel = ADC_CHANNEL_3;
static float s_rl = MQ7_RL_VALUE;
static float s_ro = MQ7_RO_CLEAN_AIR;
static bool s_initialized = false;

/**
 * @brief Initialize MQ-7 sensor
 */
esp_err_t mq7_init(adc_oneshot_unit_handle_t handle, adc1_channel_t adc_channel)
{
    s_adc_handle = handle;
    s_adc_channel = (adc_channel_t)adc_channel;
    
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
    ESP_LOGI(TAG, "MQ-7 initialized");
    
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
    float v_voltage = voltage_mv / 1000.0f;
    
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
