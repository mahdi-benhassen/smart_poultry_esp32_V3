/**
 * MQ-7 Gas Sensor Driver (Carbon Monoxide)
 */

#ifndef MQ7_H
#define MQ7_H

#include <stdint.h>
#include <esp_err.h>
#include <driver/adc.h>
#include "esp_adc/adc_oneshot.h"

/* MQ-7 typical values */
#define MQ7_RL_VALUE             10.0f   /* Load resistance in kOhm */
#define MQ7_RO_CLEAN_AIR        30.0f   /* Ro in clean air */

/**
 * @brief Initialize MQ-7 sensor
 */
esp_err_t mq7_init(adc_oneshot_unit_handle_t handle, adc1_channel_t adc_channel);

/**
 * @brief Read CO concentration in PPM
 */
esp_err_t mq7_read(float *ppm);

/**
 * @brief Calibrate sensor
 */
esp_err_t mq7_calibrate(float ro);

/**
 * @brief Get Ro value
 */
float mq7_get_ro(void);

#endif /* MQ7_H */
