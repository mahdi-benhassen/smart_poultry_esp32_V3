/**
 * Sensor Manager - Header File
 * Smart Poultry V3 - Extended Sensor Support
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <hal/adc_types.h>
#include <driver/uart.h>
#include <driver/i2c.h>

/* ADC Channels */
#define ADC_MQ135                ADC_CHANNEL_0
#define ADC_MQ7                  ADC_CHANNEL_3
#define ADC_MQ2                  ADC_CHANNEL_6
#define ADC_LDR                  ADC_CHANNEL_7
#define ADC_WATER_LEVEL          ADC_CHANNEL_4
#define ADC_SOUND                ADC_CHANNEL_5
#define ADC_ACS712               ADC_CHANNEL_6  /* Shared with MQ2 */

/* GPIO Pin Definitions for Sensors */
#define GPIO_DHT22               4
#define GPIO_DS18B20             5
#define GPIO_MQ135               36
#define GPIO_MQ7                 39
#define GPIO_MQ2                 34
#define GPIO_LDR                 35
#define GPIO_WATER_LEVEL         32
#define GPIO_SOUND               33
#define GPIO_HX711_DOUT          26  /* Weight sensor */
#define GPIO_HX711_SCK           27

/* UART Configuration for PMS5003 */
#define PMS5003_UART_NUM         UART_NUM_1
#define PMS5003_TX_PIN           15
#define PMS5003_RX_PIN           16

/* I2C Configuration for BMP280 and SGP30 */
#define SENSORS_I2C_NUM          I2C_NUM_0
#define I2C_SENSOR_SDA           21
#define I2C_SENSOR_SCL           22
#define BMP280_ADDR              0x76
#define SGP30_ADDR               0x58

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
    /* New sensors */
    SENSOR_TYPE_BMP280,         /* Barometric pressure */
    SENSOR_TYPE_SGP30,          /* CO2 & TVOC */
    SENSOR_TYPE_PMS5003,        /* Particulate matter (PM2.5/PM10) */
    SENSOR_TYPE_HX711,          /* Weight/load cell */
    SENSOR_TYPE_ACS712,         /* Current monitoring */
    SENSOR_TYPE_MAX
} sensor_type_t;

/* Air Quality Index levels */
typedef enum {
    AQI_GOOD = 0,
    AQI_MODERATE = 1,
    AQI_UNHEALTHY_SENSITIVE = 2,
    AQI_UNHEALTHY = 3,
    AQI_VERY_UNHEALTHY = 4,
    AQI_HAZARDOUS = 5
} aqi_level_t;

/* Sensor Data Structure - Optimized Union-based Structure */
typedef struct sensor_data {
    /* Common fields */
    uint32_t timestamp;

    /* Core environmental measurements (always present) */
    struct {
        float temperature;      /* Primary temperature */
        float humidity;         /* Humidity */
        uint16_t aqi_value;     /* Air Quality Index */
        aqi_level_t aqi_level;  /* AQI level */
    } core;

    /* Union for optional sensors - saves ~40 bytes when sensors are disabled */
    union {
        struct {
            /* Extended temperature/pressure */
            float temperature_ds18b20;
            float water_temperature;
            float pressure_hpa;         /* BMP280 */
            float altitude_m;           /* BMP280 */

            /* Gas sensors */
            float ammonia_ppm;
            float co_ppm;
            float co2_ppm;
            float h2s_ppm;
            float lpg_ppm;
            float tvoc_ppb;             /* SGP30 */

            /* Particulate matter */
            uint16_t pm1_0_atm;         /* PM1.0 atmospheric */
            uint16_t pm2_5_atm;         /* PM2.5 atmospheric */
            uint16_t pm10_atm;          /* PM10 atmospheric */

            /* Environmental */
            float light_percent;
            uint8_t water_level;
            float sound_level;

            /* Weight and power */
            float weight_g;             /* HX711 */
            float current_amps;          /* ACS712 */
            float power_watts;          /* ACS712 */
            float energy_wh;            /* ACS712 */
        } extended;

        /* Alternative: Compact representation when minimal sensors enabled */
        struct {
            float gas_reading;
            float environment_reading;
            float power_reading;
        } minimal;
    } data;

    /* Bitmask indicating which sensors are present/enabled */
    uint32_t sensors_present;

    /* Flags for sensor readings validity */
    uint32_t readings_valid;  /* One per sensor type */

} sensor_data_t;

/* Macros for accessing sensor data in backward-compatible way */
#define SENSOR_DATA_TEMP(sd)                     ((sd)->core.temperature)
#define SENSOR_DATA_HUMIDITY(sd)                 ((sd)->core.humidity)
#define SENSOR_DATA_TEMP_DS18B20(sd)             ((sd)->data.extended.temperature_ds18b20)
#define SENSOR_DATA_WATER_TEMP(sd)               ((sd)->data.extended.water_temperature)
#define SENSOR_DATA_PRESSURE(sd)                 ((sd)->data.extended.pressure_hpa)
#define SENSOR_DATA_ALTITUDE(sd)                 ((sd)->data.extended.altitude_m)
#define SENSOR_DATA_AMMONIA(sd)                  ((sd)->data.extended.ammonia_ppm)
#define SENSOR_DATA_CO(sd)                       ((sd)->data.extended.co_ppm)
#define SENSOR_DATA_CO2(sd)                      ((sd)->data.extended.co2_ppm)
#define SENSOR_DATA_H2S(sd)                      ((sd)->data.extended.h2s_ppm)
#define SENSOR_DATA_LPG(sd)                      ((sd)->data.extended.lpg_ppm)
#define SENSOR_DATA_TVOC(sd)                     ((sd)->data.extended.tvoc_ppb)
#define SENSOR_DATA_PM1_0(sd)                    ((sd)->data.extended.pm1_0_atm)
#define SENSOR_DATA_PM2_5(sd)                    ((sd)->data.extended.pm2_5_atm)
#define SENSOR_DATA_PM10(sd)                     ((sd)->data.extended.pm10_atm)
#define SENSOR_DATA_LIGHT(sd)                    ((sd)->data.extended.light_percent)
#define SENSOR_DATA_WATER_LEVEL(sd)              ((sd)->data.extended.water_level)
#define SENSOR_DATA_SOUND(sd)                    ((sd)->data.extended.sound_level)
#define SENSOR_DATA_WEIGHT(sd)                   ((sd)->data.extended.weight_g)
#define SENSOR_DATA_CURRENT(sd)                  ((sd)->data.extended.current_amps)
#define SENSOR_DATA_POWER(sd)                    ((sd)->data.extended.power_watts)
#define SENSOR_DATA_ENERGY(sd)                   ((sd)->data.extended.energy_wh)
#define SENSOR_DATA_AQI(sd)                      ((sd)->core.aqi_value)
#define SENSOR_DATA_AQI_LEVEL(sd)                ((sd)->core.aqi_level)

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
 * @brief Initialize all sensors (including new ones)
 */
esp_err_t sensor_manager_init(void);

/**
 * @brief Initialize new I2C sensors (BMP280, SGP30)
 */
esp_err_t sensor_manager_init_i2c_sensors(void);

/**
 * @brief Initialize UART sensors (PMS5003)
 */
esp_err_t sensor_manager_init_uart_sensors(void);

/**
 * @brief Read all sensor data
 */
esp_err_t sensor_manager_read_all(void *data);

/**
 * @brief Read single sensor
 */
esp_err_t sensor_manager_read(sensor_type_t type, float *value);

/**
 * @brief Read BMP280 (pressure/altitude)
 */
esp_err_t sensor_manager_read_pressure(float *pressure_hpa, float *altitude_m);

/**
 * @brief Read SGP30 (CO2/TVOC)
 */
esp_err_t sensor_manager_read_air_quality(float *co2_ppm, float *tvoc_ppb);

/**
 * @brief Read PMS5003 (particulate matter)
 */
esp_err_t sensor_manager_read_particulate(uint16_t *pm25, uint16_t *pm10);

/**
 * @brief Read HX711 (weight)
 */
esp_err_t sensor_manager_read_weight(float *weight_g);

/**
 * @brief Read ACS712 (current/power)
 */
esp_err_t sensor_manager_read_current(float *current_amps, float *power_watts);

/**
 * @brief Calculate Air Quality Index
 */
aqi_level_t sensor_manager_calculate_aqi(const sensor_data_t *data);

/**
 * @brief Tare (zero) weight sensor
 */
esp_err_t sensor_manager_tare_weight(uint8_t samples);

/**
 * @brief Calibrate weight sensor with known weight
 */
esp_err_t sensor_manager_calibrate_weight(float known_weight_g);

/**
 * @brief Calibrate current sensor
 */
esp_err_t sensor_manager_calibrate_current(int offset);

/**
 * @brief Get cumulative energy
 */
esp_err_t sensor_manager_get_energy(float *energy_wh);

/**
 * @brief Reset energy counter
 */
esp_err_t sensor_manager_reset_energy(void);

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
