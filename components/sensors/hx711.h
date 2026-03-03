/**
 * HX711 Weight Sensor Driver
 * For bird weight monitoring and growth tracking
 * Critical for poultry health and production management
 */

#ifndef HX711_H
#define HX711_H

#include <stdint.h>
#include <esp_err.h>
#include <driver/gpio.h>

/**
 * @brief Initialize HX711 sensor
 */
esp_err_t hx711_init(gpio_num_t dout, gpio_num_t sck);

/**
 * @brief Read raw weight value
 */
esp_err_t hx711_read_raw(int32_t *raw);

/**
 * @brief Read weight in grams
 */
esp_err_t hx711_read_weight(float *weight_g);

/**
 * @brief Set calibration factor
 */
esp_err_t hx711_set_calibration(float cal_factor);

/**
 * @brief Get calibration factor
 */
float hx711_get_calibration(void);

/**
 * @brief Tare (zero) the scale
 */
esp_err_t hx711_tare(uint8_t samples);

/**
 * @brief Set offset
 */
esp_err_t hx711_set_offset(int32_t offset);

/**
 * @brief Get offset
 */
int32_t hx711_get_offset(void);

/**
 * @brief Power down the sensor
 */
esp_err_t hx711_power_down(void);

/**
 * @brief Power up the sensor
 */
esp_err_t hx711_power_up(void);

/**
 * @brief Check if sensor is ready
 */
bool hx711_is_ready(void);

/**
 * @brief Get last known weight
 */
esp_err_t hx711_get_last_weight(float *weight_g);

#endif /* HX711_H */
