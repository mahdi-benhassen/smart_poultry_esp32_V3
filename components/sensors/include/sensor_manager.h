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

/* Sensor Data Structure - Extended for new sensors */
typedef struct sensor_data {
    /* Environmental sensors */
    float temperature;
    float humidity;
    float temperature_ds18b20;
    float water_temperature;
    float pressure_hpa;         /* BMP280 - Barometric pressure */
    float altitude_m;           /* BMP280 - Altitude */
    
    /* Gas sensors */
    float ammonia_ppm;
    float co_ppm;
    float co2_ppm;
    float h2s_ppm;
    float lpg_ppm;
    float tvoc_ppb;             /* SGP30 - Total Volatile Organic Compounds */
    
    /* Particulate matter (PMS5003) */
    uint16_t pm1_0_atm;         /* PM1.0 atmospheric */
    uint16_t pm2_5_atm;         /* PM2.5 atmospheric */
    uint16_t pm10_atm;          /* PM10 atmospheric */
    
    /* Light and power */
    float light_percent;
    uint8_t water_level;
    float sound_level;
    
    /* Weight sensor (HX711) */
    float weight_g;             /* Bird/equipment weight in grams */
    
    /* Current/Energy monitoring (ACS712) */
    float current_amps;          /* Current in Amperes */
    float power_watts;          /* Power consumption in Watts */
    float energy_wh;            /* Cumulative energy in Watt-hours */
    
    /* Air Quality Index */
    aqi_level_t aqi_level;
    uint16_t aqi_value;
    
    /* Timestamp */
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
