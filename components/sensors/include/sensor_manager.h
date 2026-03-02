/**
 * Sensor Manager - Header File
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <hal/adc_types.h>

/* ADC Channels */
#define ADC_MQ135                ADC1_CHANNEL_0
#define ADC_MQ7                  ADC1_CHANNEL_3
#define ADC_MQ2                  ADC1_CHANNEL_6
#define ADC_LDR                  ADC1_CHANNEL_7
#define ADC_WATER_LEVEL          ADC1_CHANNEL_4
#define ADC_SOUND                ADC1_CHANNEL_5

/* GPIO Pin Definitions for Sensors */
#define GPIO_DHT22               4
#define GPIO_DS18B20             5
#define GPIO_MQ135               36
#define GPIO_MQ7                 39
#define GPIO_MQ2                 34
#define GPIO_LDR                 35
#define GPIO_WATER_LEVEL         32
#define GPIO_SOUND               33

/* Sensor types */
typedef enum {
    SENSOR_TYPE_DHT22,
    SENSOR_TYPE_DS18B20,
    SENSOR_TYPE_MQ135,
    SENSOR_TYPE_MQ7,
    SENSOR_TYPE_MQ2,
    SENSOR_TYPE_LDR,
    SENSOR_TYPE_WATER_LEVEL,
    SENSOR_TYPE_SOUND,
    SENSOR_TYPE_MAX
} sensor_type_t;

/* Sensor Data Structure */
typedef struct {
    float temperature;
    float humidity;
    float temperature_ds18b20;
    float water_temperature;
    float ammonia_ppm;
    float co_ppm;
    float co2_ppm;
    float h2s_ppm;
    float lpg_ppm;
    float light_percent;
    uint8_t water_level;
    float sound_level;
    uint32_t timestamp;
} sensor_data_t;

/* Sensor reading */
typedef struct {
    sensor_type_t type;
    float value;
    uint32_t timestamp;
    bool valid;
} sensor_reading_t;

/* Sensor configuration */
typedef struct {
    gpio_num_t gpio;
    adc_channel_t adc_channel;
    float calibration_offset;
    float calibration_scale;
    bool enabled;
} sensor_config_t;

/**
 * @brief Initialize all sensors
 */
esp_err_t sensor_manager_init(void);

/**
 * @brief Read all sensor data
 */
esp_err_t sensor_manager_read_all(void *data);

/**
 * @brief Read single sensor
 */
esp_err_t sensor_manager_read(sensor_type_t type, float *value);

/**
 * @brief Calibrate sensor
 */
esp_err_t sensor_manager_calibrate(sensor_type_t type, float reference);

/**
 * @brief Get sensor configuration
 */
esp_err_t sensor_manager_get_config(sensor_type_t type, sensor_config_t *config);

/**
 * @brief Set sensor configuration
 */
esp_err_t sensor_manager_set_config(sensor_type_t type, const sensor_config_t *config);

/**
 * @brief Enable/disable sensor
 */
esp_err_t sensor_manager_enable(sensor_type_t type, bool enable);

/**
 * @brief Self-test sensors
 */
esp_err_t sensor_manager_self_test(void);

#endif /* SENSOR_MANAGER_H */
