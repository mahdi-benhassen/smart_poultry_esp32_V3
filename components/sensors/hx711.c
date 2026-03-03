/**
 * HX711 Weight Sensor Driver - Implementation
 * For bird weight monitoring and growth tracking
 * Critical for poultry health and production management
 */

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <rom/ets_sys.h>
#include "hx711.h"

static const char *TAG = "HX711";

/* HX711 Timing */
#define HX711_PULSE_US           1
#define HX711_TIMEOUT_MS         100

/* HX711 Gain Settings */
#define HX711_GAIN_128           1
#define HX711_GAIN_64            3
#define HX711_GAIN_32            2

/* Global state */
static gpio_num_t s_dout = GPIO_NUM_NC;
static gpio_num_t s_sck = GPIO_NUM_NC;
static bool s_initialized = false;
static float s_calibration = -7050.0f;  /* Default calibration factor */
static int32_t s_offset = 0;
static float s_last_weight = 0.0f;
static uint8_t s_gain = HX711_GAIN_128;

/**
 * @brief Wait for sensor to be ready
 */
static bool hx711_wait_ready(uint32_t timeout_ms)
{
    uint32_t start = esp_log_timestamp();
    
    while (gpio_get_level(s_dout) == 1) {
        if (esp_log_timestamp() - start > timeout_ms) {
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Pulse the clock line
 */
static void hx711_pulse_clock(void)
{
    gpio_set_level(s_sck, 1);
    ets_delay_us(HX711_PULSE_US);
    gpio_set_level(s_sck, 0);
    ets_delay_us(HX711_PULSE_US);
}

/**
 * @brief Initialize HX711 sensor
 */
esp_err_t hx711_init(gpio_num_t dout, gpio_num_t sck)
{
    s_dout = dout;
    s_sck = sck;
    
    /* Configure SCK as output */
    gpio_config_t sck_conf = {
        .pin_bit_mask = (1ULL << sck),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&sck_conf));
    
    /* Configure DOUT as input */
    gpio_config_t dout_conf = {
        .pin_bit_mask = (1ULL << dout),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&dout_conf));
    
    /* Initialize SCK to low */
    gpio_set_level(s_sck, 0);
    
    s_initialized = true;
    ESP_LOGI(TAG, "HX711 initialized: DOUT=GPIO%d, SCK=GPIO%d", dout, sck);
    
    /* Read once to initialize */
    int32_t raw;
    hx711_read_raw(&raw);
    
    return ESP_OK;
}

/**
 * @brief Read raw value
 */
esp_err_t hx711_read_raw(int32_t *raw)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (raw == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* Wait for sensor to be ready */
    if (!hx711_wait_ready(HX711_TIMEOUT_MS)) {
        ESP_LOGW(TAG, "HX711 not ready (timeout)");
        return ESP_ERR_TIMEOUT;
    }
    
    /* Read 24 bits */
    int32_t value = 0;
    for (int i = 0; i < 24; i++) {
        hx711_pulse_clock();
        value = (value << 1) | gpio_get_level(s_dout);
    }
    
    /* Set gain and read additional bits */
    for (int i = 0; i < s_gain; i++) {
        hx711_pulse_clock();
    }
    
    /* Convert from 24-bit signed to 32-bit signed */
    if (value & 0x800000) {
        value |= 0xFF000000;
    }
    
    *raw = value;
    
    return ESP_OK;
}

/**
 * @brief Read weight in grams
 */
esp_err_t hx711_read_weight(float *weight_g)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (weight_g == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int32_t raw;
    esp_err_t ret = hx711_read_raw(&raw);
    if (ret != ESP_OK) {
        return ret;
    }
    
    /* Apply offset and calibration */
    float weight = ((float)(raw - s_offset) / s_calibration);
    
    /* Filter unrealistic values */
    if (weight < -5000.0f || weight > 50000.0f) {
        ESP_LOGW(TAG, "Unrealistic weight: %.1fg (raw: %ld)", weight, raw);
        *weight_g = s_last_weight;  /* Return last known value */
        return ESP_OK;
    }
    
    s_last_weight = weight;
    *weight_g = weight;
    
    ESP_LOGD(TAG, "Weight: %.1fg (raw: %ld, offset: %ld, cal: %.2f)", 
             weight, raw, s_offset, s_calibration);
    
    return ESP_OK;
}

/**
 * @brief Set calibration factor
 */
esp_err_t hx711_set_calibration(float cal_factor)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    s_calibration = cal_factor;
    ESP_LOGI(TAG, "Calibration factor set to: %.2f", cal_factor);
    
    return ESP_OK;
}

/**
 * @brief Get calibration factor
 */
float hx711_get_calibration(void)
{
    return s_calibration;
}

/**
 * @brief Tare (zero) the scale
 */
esp_err_t hx711_tare(uint8_t samples)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (samples == 0) {
        samples = 10;
    }
    
    int64_t sum = 0;
    
    for (int i = 0; i < samples; i++) {
        int32_t raw;
        esp_err_t ret = hx711_read_raw(&raw);
        if (ret != ESP_OK) {
            return ret;
        }
        sum += raw;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    s_offset = (int32_t)(sum / samples);
    ESP_LOGI(TAG, "Tare complete: offset=%ld (%d samples)", s_offset, samples);
    
    return ESP_OK;
}

/**
 * @brief Set offset
 */
esp_err_t hx711_set_offset(int32_t offset)
{
    s_offset = offset;
    return ESP_OK;
}

/**
 * @brief Get offset
 */
int32_t hx711_get_offset(void)
{
    return s_offset;
}

/**
 * @brief Power down the sensor
 */
esp_err_t hx711_power_down(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    gpio_set_level(s_sck, 0);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(s_sck, 1);
    
    ESP_LOGD(TAG, "HX711 powered down");
    return ESP_OK;
}

/**
 * @brief Power up the sensor
 */
esp_err_t hx711_power_up(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    gpio_set_level(s_sck, 0);
    vTaskDelay(pdMS_TO_TICKS(100));  /* Wait for startup */
    
    /* Read to initialize */
    int32_t raw;
    hx711_read_raw(&raw);
    
    ESP_LOGD(TAG, "HX711 powered up");
    return ESP_OK;
}

/**
 * @brief Check if sensor is ready
 */
bool hx711_is_ready(void)
{
    if (!s_initialized) {
        return false;
    }
    
    return gpio_get_level(s_dout) == 0;
}

/**
 * @brief Get last known weight
 */
esp_err_t hx711_get_last_weight(float *weight_g)
{
    if (weight_g == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *weight_g = s_last_weight;
    return ESP_OK;
}
