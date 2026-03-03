/**
 * BMP280 Barometric Pressure Sensor Driver
 * For ventilation optimization and weather monitoring
 */

#ifndef BMP280_H
#define BMP280_H

#include <stdint.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c.h>

/**
 * @brief Initialize BMP280 sensor
 */
esp_err_t bmp280_init(i2c_port_t i2c_port, uint8_t i2c_addr);

/**
 * @brief Read temperature and pressure
 */
esp_err_t bmp280_read(float *temperature, float *pressure_hpa);

/**
 * @brief Read only pressure
 */
esp_err_t bmp280_read_pressure(float *pressure_hpa);

/**
 * @brief Read altitude based on sea level pressure
 */
esp_err_t bmp280_read_altitude(float sea_level_hpa, float *altitude_m);

/**
 * @brief Set sea level pressure for altitude calculation
 */
esp_err_t bmp280_set_sea_level_pressure(float pressure_hpa);

/**
 * @brief Get sensor calibration data
 */
esp_err_t bmp280_get_calibration(uint16_t *dig_T1, int16_t *dig_T2, int16_t *dig_T3,
                                  uint16_t *dig_P1, int16_t *dig_P2, int16_t *dig_P3,
                                  int16_t *dig_P4, int16_t *dig_P5, int16_t *dig_P6,
                                  int16_t *dig_P7, int16_t *dig_P8, int16_t *dig_P9);

#endif /* BMP280_H */
