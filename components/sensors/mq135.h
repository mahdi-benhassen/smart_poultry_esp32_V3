/**
 * MQ-135 Gas Sensor Driver (Ammonia/CO2)
 */

#ifndef MQ135_H
#define MQ135_H

#include <stdint.h>
#include <esp_err.h>
#include <driver/adc.h>
#include "esp_adc/adc_oneshot.h"

/* MQ-135 typical values */
#define MQ135_RL_VALUE          10.0f   /* Load resistance in kOhm */
#define MQ135_RO_CLEAN_AIR       60.0f   /* Ro in clean air */

/**
 * @brief Initialize MQ-135 sensor
 */
esp_err_t mq135_init(adc_oneshot_unit_handle_t handle, adc1_channel_t adc_channel);

/**
 * @brief Read gas concentration in PPM
 */
esp_err_t mq135_read(float *ppm);

/**
 * @brief Calibrate sensor (run in clean air for 24h first)
 */
esp_err_t mq135_calibrate(float ro);

/**
 * @brief Get Ro value
 */
float mq135_get_ro(void);

/**
 * @brief Set RL value
 */
void mq135_set_rl(float rl);

/**
 * @brief Enable/disable heating
 */
esp_err_t mq135_heater_enable(bool enable);

/**
 * @brief Deinitialize MQ-135 sensor
 */
esp_err_t mq135_deinit(void);

#endif /* MQ135_H */
