/**
 * Sound Sensor Driver
 */

#ifndef SOUND_SENSOR_H
#define SOUND_SENSOR_H

#include <stdint.h>
#include <esp_err.h>
#include <driver/adc.h>

/**
 * @brief Initialize sound sensor
 */
esp_err_t sound_sensor_init(adc1_channel_t adc_channel);

/**
 * @brief Read sound level (0-100%)
 */
esp_err_t sound_sensor_read(float *level);

/**
 * @brief Get average sound level
 */
esp_err_t sound_sensor_get_average(float *level);

#endif /* SOUND_SENSOR_H */
