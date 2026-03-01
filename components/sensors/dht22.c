/**
 * DHT22 Temperature & Humidity Sensor Driver - Implementation
 */

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "dht22.h"

static const char *TAG = "DHT22";

/* DHT22 timing constants */
#define DHT22_TIMEOUT_US         100
#define DHT22_START_DELAY_MS     18
#define DHT22_BIT_COUNT          40

/* Global state */
static gpio_num_t s_gpio = GPIO_DHT22;
static bool s_initialized = false;
static uint32_t s_interval_ms = 2000;  /* Default 2 second interval */
static uint32_t s_last_read_time = 0;

/**
 * @brief Initialize DHT22 sensor
 */
esp_err_t dht22_init(gpio_num_t gpio)
{
    s_gpio = gpio;
    
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    
    s_initialized = true;
    ESP_LOGI(TAG, "DHT22 initialized on GPIO%d", gpio);
    
    return ESP_OK;
}

/**
 * @brief Read temperature and humidity
 */
esp_err_t dht22_read(float *temperature, float *humidity)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (temperature == NULL || humidity == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* Check interval since last reading */
    uint32_t now = esp_log_timestamp();
    if (s_last_read_time > 0 && (now - s_last_read_time) < s_interval_ms) {
        /* Too soon since last reading, return cached/safe values */
        *temperature = 0;
        *humidity = 0;
        return ESP_ERR_TIMEOUT;
    }
    
    uint8_t data[5] = {0};
    int64_t start_time, end_time;
    int bit_count = 0;
    
    /* Send start signal */
    gpio_set_direction(s_gpio, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(s_gpio, 0);
    vTaskDelay(pdMS_TO_TICKS(DHT22_START_DELAY_MS));
    gpio_set_level(s_gpio, 1);
    
    /* Wait for DHT22 response */
    gpio_set_direction(s_gpio, GPIO_MODE_INPUT);
    
    /* Check response: 80us low, 80us high */
    start_time = esp_timer_get_time();
    while (gpio_get_level(s_gpio) == 0) {
        if (esp_timer_get_time() - start_time > DHT22_TIMEOUT_US * 2) {
            ESP_LOGE(TAG, "DHT22 response timeout (low)");
            return ESP_ERR_TIMEOUT;
        }
    }
    
    start_time = esp_timer_get_time();
    while (gpio_get_level(s_gpio) == 1) {
        if (esp_timer_get_time() - start_time > DHT22_TIMEOUT_US * 2) {
            ESP_LOGE(TAG, "DHT22 response timeout (high)");
            return ESP_ERR_TIMEOUT;
        }
    }
    
    /* Read 40 bits */
    for (int i = 0; i < DHT22_BIT_COUNT; i++) {
        /* Start of bit: 50us low */
        start_time = esp_timer_get_time();
        while (gpio_get_level(s_gpio) == 0) {
            if (esp_timer_get_time() - start_time > DHT22_TIMEOUT_US) {
                ESP_LOGE(TAG, "DHT22 bit timeout (low)");
                return ESP_ERR_TIMEOUT;
            }
        }
        
        /* Read bit: measure high duration */
        start_time = esp_timer_get_time();
        end_time = start_time;
        while (gpio_get_level(s_gpio) == 1) {
            end_time = esp_timer_get_time();
            if (end_time - start_time > DHT22_TIMEOUT_US) {
                ESP_LOGE(TAG, "DHT22 bit timeout (high)");
                return ESP_ERR_TIMEOUT;
            }
        }
        
        /* Determine bit value */
        if (end_time - start_time > 30) {
            data[bit_count / 8] |= (1 << (7 - (bit_count % 8)));
        }
        bit_count++;
    }
    
    /* Verify checksum */
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        ESP_LOGE(TAG, "DHT22 checksum error: expected %02x, got %02x", 
                 checksum, data[4]);
        return ESP_ERR_INVALID_CRC;
    }
    
    /* Parse data */
    uint16_t raw_humidity = (data[0] << 8) | data[1];
    uint16_t raw_temperature = (data[2] << 8) | data[3];
    
    /* Handle negative temperature */
    if (raw_temperature & 0x8000) {
        raw_temperature &= 0x7FFF;
        *temperature = -(float)raw_temperature / 10.0f;
    } else {
        *temperature = (float)raw_temperature / 10.0f;
    }
    
    *humidity = (float)raw_humidity / 10.0f;
    
    /* Update last read time */
    s_last_read_time = esp_log_timestamp();
    
    ESP_LOGD(TAG, "DHT22: Temp=%.1fC, Humidity=%.1f%%", *temperature, *humidity);
    
    return ESP_OK;
}

/**
 * @brief Set measurement interval
 */
esp_err_t dht22_set_interval(uint32_t interval_ms)
{
    /* DHT22 requires minimum 2 seconds between readings */
    if (interval_ms < 2000) {
        s_interval_ms = 2000;
    } else {
        s_interval_ms = interval_ms;
    }
    ESP_LOGI(TAG, "DHT22 read interval set to %ums", s_interval_ms);
    return ESP_OK;
}
