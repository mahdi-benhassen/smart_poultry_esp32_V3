/**
 * MQ-2 Gas Sensor Driver (LPG/Propane/Smoke)
 */

#ifndef MQ2_H
#define MQ2_H

#include <stdint.h>
#include <esp_err.h>
#include "esp_adc/adc_oneshot.h"

/* MQ-2 typical values */
#define MQ2_RL_VALUE             10.0f   /* Load resistance in kOhm */
#define MQ2_RO_CLEAN_AIR         10.0f   /* Ro in clean air */

/**
 * @brief Initialize MQ-2 sensor
 */
esp_err_t mq2_init(adc_oneshot_unit_handle_t handle, adc_channel_t adc_channel);

/**
 * @brief Read LPG concentration in PPM
 */
esp_err_t mq2_read(float *ppm);

/**
 * @brief Calibrate sensor
 */
esp_err_t mq2_calibrate(float ro);

/**
 * @brief Get Ro value
 */
float mq2_get_ro(void);

#endif /* MQ2_H */
