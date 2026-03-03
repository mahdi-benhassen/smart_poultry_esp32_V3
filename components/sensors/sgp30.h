/**
 * SGP30 Multi-Pixel Gas Sensor Driver
 * Measures CO2 equivalent and TVOC (Total Volatile Organic Compounds)
 * Critical for poultry house air quality monitoring
 */

#ifndef SGP30_H
#define SGP30_H

#include <stdint.h>
#include <esp_err.h>
#include <driver/i2c.h>

/**
 * @brief Initialize SGP30 sensor
 */
esp_err_t sgp30_init(i2c_port_t i2c_port);

/**
 * @brief Read CO2 equivalent and TVOC
 */
esp_err_t sgp30_read(uint16_t *co2_eq_ppm, uint16_t *tvoc_ppb);

/**
 * @brief Read only CO2 equivalent
 */
esp_err_t sgp30_read_co2(uint16_t *co2_eq_ppm);

/**
 * @brief Read only TVOC
 */
esp_err_t sgp30_read_tvoc(uint16_t *tvoc_ppb);

/**
 * @brief Trigger IAQ (Indoor Air Quality) measurement
 */
esp_err_t sgp30_measure(void);

/**
 * @brief Get baseline for IAQ accuracy
 */
esp_err_t sgp30_get_baseline(uint16_t *co2_baseline, uint16_t *tvoc_baseline);

/**
 * @brief Set baseline for IAQ accuracy
 */
esp_err_t sgp30_set_baseline(uint16_t co2_baseline, uint16_t tvoc_baseline);

/**
 * @brief Set humidity for compensation
 */
esp_err_t sgp30_set_humidity(uint32_t absolute_humidity);

/**
 * * @brief Perform self-test
 */
esp_err_t sgp30_self_test(uint8_t *feature_set_version);

/**
 * @brief Get feature set version
 */
esp_err_t sgp30_get_feature_set_version(uint8_t *version);

/**
 * @brief Get unique ID
 */
esp_err_t sgp30_get_unique_id(uint64_t *unique_id);

#endif /* SGP30_H */
