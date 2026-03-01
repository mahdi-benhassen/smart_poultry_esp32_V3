/**
 * DHT22 Temperature & Humidity Sensor Driver
 */

#ifndef DHT22_H
#define DHT22_H

#include <stdint.h>
#include <esp_err.h>
#include <driver/gpio.h>

/**
 * @brief Initialize DHT22 sensor
 */
esp_err_t dht22_init(gpio_num_t gpio);

/**
 * @brief Read temperature and humidity
 */
esp_err_t dht22_read(float *temperature, float *humidity);

/**
 * @brief Set measurement interval
 */
esp_err_t dht22_set_interval(uint32_t interval_ms);

/**
 * @brief DHT22 data structure
 */
typedef struct {
    float temperature;
    float humidity;
    uint16_t raw_humidity;
    uint16_t raw_temperature;
    uint8_t checksum;
} dht22_data_t;

#endif /* DHT22_H */
