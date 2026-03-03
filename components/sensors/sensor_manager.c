/**
 * Sensor Manager - Implementation
 * Smart Poultry V3 - Extended Sensor Support
 * Modular architecture - sensors can be enabled/disabled via Kconfig
 */

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "sensor_manager.h"

#ifdef CONFIG_ENABLE_SENSOR_DHT22
#include "dht22.h"
#endif

#ifdef CONFIG_ENABLE_SENSOR_DS18B20
#include "ds18b20.h"
#endif

#ifdef CONFIG_ENABLE_SENSOR_MQ135
#include "mq135.h"
#endif

#ifdef CONFIG_ENABLE_SENSOR_MQ7
#include "mq7.h"
#endif

#ifdef CONFIG_ENABLE_SENSOR_MQ2
#include "mq2.h"
#endif

#ifdef CONFIG_ENABLE_SENSOR_LDR
#include "ldr.h"
#endif

#ifdef CONFIG_ENABLE_SENSOR_WATER_LEVEL
#include "water_level.h"
#endif

#ifdef CONFIG_ENABLE_SENSOR_SOUND
#include "sound_sensor.h"
#endif

#ifdef CONFIG_ENABLE_SENSOR_BMP280
#include "bmp280.h"
#endif

#ifdef CONFIG_ENABLE_SENSOR_SGP30
#include "sgp30.h"
#endif

#ifdef CONFIG_ENABLE_SENSOR_PMS5003
#include "pms5003.h"
#endif

#ifdef CONFIG_ENABLE_SENSOR_HX711
#include "hx711.h"
#endif

#ifdef CONFIG_ENABLE_SENSOR_ACS712
#include "acs712.h"
#endif

static const char *TAG = "SENSOR_MGR";

/* ADC Handle */
static adc_oneshot_unit_handle_t s_adc_handle = NULL;

/* I2C handle */
static bool s_i2c_initialized = false;

/* Sensor configurations */
static sensor_config_t sensor_configs[SENSOR_TYPE_MAX] = {
#ifdef CONFIG_ENABLE_SENSOR_DHT22
    [SENSOR_TYPE_DHT22] = {
        .gpio = GPIO_DHT22,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
#endif
#ifdef CONFIG_ENABLE_SENSOR_DS18B20
    [SENSOR_TYPE_DS18B20] = {
        .gpio = GPIO_DS18B20,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
#endif
#ifdef CONFIG_ENABLE_SENSOR_MQ135
    [SENSOR_TYPE_MQ135] = {
        .gpio = GPIO_MQ135,
        .adc_channel = ADC_MQ135,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
#endif
#ifdef CONFIG_ENABLE_SENSOR_MQ7
    [SENSOR_TYPE_MQ7] = {
        .gpio = GPIO_MQ7,
        .adc_channel = ADC_MQ7,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
#endif
#ifdef CONFIG_ENABLE_SENSOR_MQ2
    [SENSOR_TYPE_MQ2] = {
        .gpio = GPIO_MQ2,
        .adc_channel = ADC_MQ2,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
#endif
#ifdef CONFIG_ENABLE_SENSOR_LDR
    [SENSOR_TYPE_LDR] = {
        .gpio = GPIO_LDR,
        .adc_channel = ADC_LDR,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
#endif
#ifdef CONFIG_ENABLE_SENSOR_WATER_LEVEL
    [SENSOR_TYPE_WATER_LEVEL] = {
        .gpio = GPIO_WATER_LEVEL,
        .adc_channel = ADC_WATER_LEVEL,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
#endif
#ifdef CONFIG_ENABLE_SENSOR_SOUND
    [SENSOR_TYPE_SOUND] = {
        .gpio = GPIO_SOUND,
        .adc_channel = ADC_SOUND,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
#endif
#ifdef CONFIG_ENABLE_SENSOR_BMP280
    [SENSOR_TYPE_BMP280] = {
        .gpio = GPIO_NUM_NC,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
#endif
#ifdef CONFIG_ENABLE_SENSOR_SGP30
    [SENSOR_TYPE_SGP30] = {
        .gpio = GPIO_NUM_NC,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
#endif
#ifdef CONFIG_ENABLE_SENSOR_PMS5003
    [SENSOR_TYPE_PMS5003] = {
        .gpio = GPIO_NUM_NC,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
#endif
#ifdef CONFIG_ENABLE_SENSOR_HX711
    [SENSOR_TYPE_HX711] = {
        .gpio = GPIO_HX711_DOUT,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
#endif
#ifdef CONFIG_ENABLE_SENSOR_ACS712
    [SENSOR_TYPE_ACS712] = {
        .gpio = GPIO_NUM_NC,
        .adc_channel = ADC_ACS712,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    }
#endif
};

/* Last sensor readings */
static float last_readings[SENSOR_TYPE_MAX] = {0};
static uint32_t last_timestamps[SENSOR_TYPE_MAX] = {0};
static bool readings_valid[SENSOR_TYPE_MAX] = {false};

/**
 * @brief Initialize I2C for BMP280 and SGP30
 */
esp_err_t sensor_manager_init_i2c_sensors(void)
{
    if (s_i2c_initialized) {
        return ESP_OK;
    }
    
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SENSOR_SDA,
        .scl_io_num = I2C_SENSOR_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    
    esp_err_t ret = i2c_param_config(SENSORS_I2C_NUM, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2C");
        return ret;
    }
    
    ret = i2c_driver_install(SENSORS_I2C_NUM, I2C_MODE_MASTER, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2C driver");
        return ret;
    }
    
    s_i2c_initialized = true;
    ESP_LOGI(TAG, "I2C initialized for sensors");
    
    return ESP_OK;
}

/**
 * @brief Initialize UART for PMS5003
 */
esp_err_t sensor_manager_init_uart_sensors(void)
{
#ifdef CONFIG_ENABLE_SENSOR_PMS5003
    ESP_LOGI(TAG, "PMS5003 UART config: UART%d TX=%d RX=%d", 
             PMS5003_UART_NUM, PMS5003_TX_PIN, PMS5003_RX_PIN);
#endif
    return ESP_OK;
}

/**
 * @brief Initialize all sensors
 */
esp_err_t sensor_manager_init(void)
{
    esp_err_t ret = ESP_OK;
    
    ESP_LOGI(TAG, "Initializing sensors (Smart Poultry V3 Extended - Modular)...");
    
    /* Initialize ADC Oneshot Unit */
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ret = adc_oneshot_new_unit(&init_config, &s_adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create ADC oneshot unit: %s", esp_err_to_name(ret));
        return ret;
    }
    
    /* Config ADC Channels */
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    
#ifdef CONFIG_ENABLE_SENSOR_MQ135
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_MQ135, &config));
#endif
#ifdef CONFIG_ENABLE_SENSOR_MQ7
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_MQ7, &config));
#endif
#ifdef CONFIG_ENABLE_SENSOR_MQ2
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_MQ2, &config));
#endif
#ifdef CONFIG_ENABLE_SENSOR_LDR
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_LDR, &config));
#endif
#ifdef CONFIG_ENABLE_SENSOR_WATER_LEVEL
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_WATER_LEVEL, &config));
#endif
#ifdef CONFIG_ENABLE_SENSOR_SOUND
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_SOUND, &config));
#endif
    
#ifdef CONFIG_ENABLE_SENSOR_DHT22
    ret = dht22_init(sensor_configs[SENSOR_TYPE_DHT22].gpio);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize DHT22: %s", esp_err_to_name(ret));
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_DS18B20
    ret = ds18b20_init(sensor_configs[SENSOR_TYPE_DS18B20].gpio);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize DS18B20: %s", esp_err_to_name(ret));
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_MQ135
    ret = mq135_init(s_adc_handle, sensor_configs[SENSOR_TYPE_MQ135].adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MQ135: %s", esp_err_to_name(ret));
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_MQ7
    ret = mq7_init(s_adc_handle, sensor_configs[SENSOR_TYPE_MQ7].adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MQ7: %s", esp_err_to_name(ret));
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_MQ2
    ret = mq2_init(s_adc_handle, sensor_configs[SENSOR_TYPE_MQ2].adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MQ2: %s", esp_err_to_name(ret));
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_LDR
    ret = ldr_init(s_adc_handle, sensor_configs[SENSOR_TYPE_LDR].adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LDR: %s", esp_err_to_name(ret));
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_WATER_LEVEL
    ret = water_level_init(s_adc_handle, sensor_configs[SENSOR_TYPE_WATER_LEVEL].adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Water Level sensor: %s", esp_err_to_name(ret));
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_SOUND
    ret = sound_sensor_init(s_adc_handle, sensor_configs[SENSOR_TYPE_SOUND].adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Sound sensor: %s", esp_err_to_name(ret));
    }
#endif

#if defined(CONFIG_ENABLE_SENSOR_BMP280) || defined(CONFIG_ENABLE_SENSOR_SGP30)
    ret = sensor_manager_init_i2c_sensors();
    if (ret == ESP_OK) {
#ifdef CONFIG_ENABLE_SENSOR_BMP280
        ret = bmp280_init(SENSORS_I2C_NUM, BMP280_ADDR);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize BMP280: %s", esp_err_to_name(ret));
        }
#endif
        
#ifdef CONFIG_ENABLE_SENSOR_SGP30
        ret = sgp30_init(SENSORS_I2C_NUM);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize SGP30: %s", esp_err_to_name(ret));
        }
#endif
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_PMS5003
    sensor_manager_init_uart_sensors();
    ret = pms5003_init(PMS5003_UART_NUM, PMS5003_TX_PIN, PMS5003_RX_PIN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize PMS5003: %s", esp_err_to_name(ret));
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_HX711
    ret = hx711_init(GPIO_HX711_DOUT, GPIO_HX711_SCK);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize HX711: %s", esp_err_to_name(ret));
    } else {
        hx711_tare(10);
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_ACS712
    ret = acs712_init(ACS712_20A, ADC_ACS712);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ACS712: %s", esp_err_to_name(ret));
    }
#endif

    ESP_LOGI(TAG, "Sensor initialization complete (Extended V3 - Modular)");
    
    return ESP_OK;
}

/**
 * @brief Calculate Air Quality Index
 */
aqi_level_t sensor_manager_calculate_aqi(const sensor_data_t *data)
{
    if (data == NULL) {
        return AQI_GOOD;
    }
    
    float aqi = 0.0f;
    
    if (data->pm2_5_atm > 0) {
        if (data->pm2_5_atm <= 35) {
            aqi += (data->pm2_5_atm / 35.0f) * 25;
        } else if (data->pm2_5_atm <= 55) {
            aqi += 25 + ((data->pm2_5_atm - 35) / 20.0f) * 25;
        } else if (data->pm2_5_atm <= 150) {
            aqi += 50 + ((data->pm2_5_atm - 55) / 95.0f) * 50;
        } else {
            aqi += 100 + ((data->pm2_5_atm - 150) / 850.0f) * 100;
        }
    }
    
    if (data->co2_ppm > 1000) {
        aqi += ((data->co2_ppm - 1000) / 4000.0f) * 30;
    }
    
    if (data->ammonia_ppm > 10) {
        aqi += ((data->ammonia_ppm - 10) / 40.0f) * 40;
    }
    
    if (data->tvoc_ppb > 200) {
        aqi += ((data->tvoc_ppb - 200) / 5800.0f) * 30;
    }
    
    if (aqi > 500) aqi = 500;
    
    if (aqi <= 50) return AQI_GOOD;
    else if (aqi <= 100) return AQI_MODERATE;
    else if (aqi <= 150) return AQI_UNHEALTHY_SENSITIVE;
    else if (aqi <= 200) return AQI_UNHEALTHY;
    else if (aqi <= 300) return AQI_VERY_UNHEALTHY;
    else return AQI_HAZARDOUS;
}

/**
 * @brief Read all sensor data
 */
esp_err_t sensor_manager_read_all(void *data)
{
    sensor_data_t *sensor_data = (sensor_data_t *)data;
    esp_err_t ret;
    uint32_t timestamp = esp_log_timestamp();
    
    memset(sensor_data, 0, sizeof(sensor_data_t));
    sensor_data->timestamp = timestamp;
    
#ifdef CONFIG_ENABLE_SENSOR_DHT22
    ret = dht22_read(&sensor_data->temperature, &sensor_data->humidity);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_DHT22] = sensor_data->temperature;
        last_timestamps[SENSOR_TYPE_DHT22] = timestamp;
        readings_valid[SENSOR_TYPE_DHT22] = true;
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_DS18B20
    ret = ds18b20_read(&sensor_data->temperature_ds18b20);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_DS18B20] = sensor_data->temperature_ds18b20;
        last_timestamps[SENSOR_TYPE_DS18B20] = timestamp;
        readings_valid[SENSOR_TYPE_DS18B20] = true;
    }
#endif

    if (readings_valid[SENSOR_TYPE_DHT22]) {
        sensor_data->temperature = sensor_data->temperature;
    } else if (readings_valid[SENSOR_TYPE_DS18B20]) {
        sensor_data->temperature = sensor_data->temperature_ds18b20;
    }
    
#ifdef CONFIG_ENABLE_SENSOR_BMP280
    ret = bmp280_read(&sensor_data->temperature, &sensor_data->pressure_hpa);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_BMP280] = sensor_data->pressure_hpa;
        readings_valid[SENSOR_TYPE_BMP280] = true;
        bmp280_read_altitude(1013.25f, &sensor_data->altitude_m);
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_MQ135
    ret = mq135_read(&sensor_data->ammonia_ppm);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_MQ135] = sensor_data->ammonia_ppm;
        last_timestamps[SENSOR_TYPE_MQ135] = timestamp;
        readings_valid[SENSOR_TYPE_MQ135] = true;
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_MQ7
    ret = mq7_read(&sensor_data->co_ppm);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_MQ7] = sensor_data->co_ppm;
        last_timestamps[SENSOR_TYPE_MQ7] = timestamp;
        readings_valid[SENSOR_TYPE_MQ7] = true;
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_MQ2
    ret = mq2_read(&sensor_data->lpg_ppm);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_MQ2] = sensor_data->lpg_ppm;
        last_timestamps[SENSOR_TYPE_MQ2] = timestamp;
        readings_valid[SENSOR_TYPE_MQ2] = true;
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_SGP30
    uint16_t co2_ppm, tvoc_ppb;
    ret = sgp30_read(&co2_ppm, &tvoc_ppb);
    if (ret == ESP_OK) {
        sensor_data->co2_ppm = (float)co2_ppm;
        sensor_data->tvoc_ppb = (float)tvoc_ppb;
        readings_valid[SENSOR_TYPE_SGP30] = true;
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_PMS5003
    pms5003_data_t pm_data;
    ret = pms5003_read(&pm_data);
    if (ret == ESP_OK) {
        sensor_data->pm1_0_atm = pm_data.pm1_0_atm;
        sensor_data->pm2_5_atm = pm_data.pm2_5_atm;
        sensor_data->pm10_atm = pm_data.pm10_atm;
        readings_valid[SENSOR_TYPE_PMS5003] = true;
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_LDR
    ret = ldr_read(&sensor_data->light_percent);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_LDR] = sensor_data->light_percent;
        last_timestamps[SENSOR_TYPE_LDR] = timestamp;
        readings_valid[SENSOR_TYPE_LDR] = true;
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_WATER_LEVEL
    ret = water_level_read(&sensor_data->water_level);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_WATER_LEVEL] = sensor_data->water_level;
        last_timestamps[SENSOR_TYPE_WATER_LEVEL] = timestamp;
        readings_valid[SENSOR_TYPE_WATER_LEVEL] = true;
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_SOUND
    ret = sound_sensor_read(&sensor_data->sound_level);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_SOUND] = sensor_data->sound_level;
        last_timestamps[SENSOR_TYPE_SOUND] = timestamp;
        readings_valid[SENSOR_TYPE_SOUND] = true;
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_HX711
    ret = hx711_read_weight(&sensor_data->weight_g);
    if (ret == ESP_OK) {
        readings_valid[SENSOR_TYPE_HX711] = true;
    }
#endif

#ifdef CONFIG_ENABLE_SENSOR_ACS712
    ret = acs712_read_current(&sensor_data->current_amps);
    if (ret == ESP_OK) {
        sensor_data->power_watts = sensor_data->current_amps * 220.0f;
        acs712_update_energy();
        sensor_data->energy_wh = acs712_get_energy_wh();
        readings_valid[SENSOR_TYPE_ACS712] = true;
    }
#endif

    sensor_data->aqi_level = sensor_manager_calculate_aqi(sensor_data);
    
    return ESP_OK;
}

/**
 * @brief Read single sensor
 */
esp_err_t sensor_manager_read(sensor_type_t type, float *value)
{
    esp_err_t ret = ESP_ERR_NOT_FOUND;
    
    if (type >= SENSOR_TYPE_MAX || value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    switch (type) {
#ifdef CONFIG_ENABLE_SENSOR_DHT22
        case SENSOR_TYPE_DHT22:
            ret = dht22_read(value, value + 1);
            if (ret == ESP_OK) *value = *(value + 1);
            break;
#endif
#ifdef CONFIG_ENABLE_SENSOR_DS18B20
        case SENSOR_TYPE_DS18B20:
            ret = ds18b20_read(value);
            break;
#endif
#ifdef CONFIG_ENABLE_SENSOR_MQ135
        case SENSOR_TYPE_MQ135:
            ret = mq135_read(value);
            break;
#endif
#ifdef CONFIG_ENABLE_SENSOR_MQ7
        case SENSOR_TYPE_MQ7:
            ret = mq7_read(value);
            break;
#endif
#ifdef CONFIG_ENABLE_SENSOR_MQ2
        case SENSOR_TYPE_MQ2:
            ret = mq2_read(value);
            break;
#endif
#ifdef CONFIG_ENABLE_SENSOR_LDR
        case SENSOR_TYPE_LDR:
            ret = ldr_read(value);
            break;
#endif
#ifdef CONFIG_ENABLE_SENSOR_WATER_LEVEL
        case SENSOR_TYPE_WATER_LEVEL:
            ret = water_level_read((uint8_t *)value);
            if (ret == ESP_OK) *value = *(uint8_t *)value;
            break;
#endif
#ifdef CONFIG_ENABLE_SENSOR_SOUND
        case SENSOR_TYPE_SOUND:
            ret = sound_sensor_read(value);
            break;
#endif
#ifdef CONFIG_ENABLE_SENSOR_BMP280
        case SENSOR_TYPE_BMP280:
            ret = bmp280_read_pressure(value);
            break;
#endif
#ifdef CONFIG_ENABLE_SENSOR_SGP30
        case SENSOR_TYPE_SGP30: {
            uint16_t co2, tvoc;
            ret = sgp30_read(&co2, &tvoc);
            if (ret == ESP_OK) *value = (float)co2;
            break;
        }
#endif
#ifdef CONFIG_ENABLE_SENSOR_PMS5003
        case SENSOR_TYPE_PMS5003:
            ret = pms5003_read_pm25((uint16_t *)value);
            break;
#endif
#ifdef CONFIG_ENABLE_SENSOR_HX711
        case SENSOR_TYPE_HX711:
            ret = hx711_read_weight(value);
            break;
#endif
#ifdef CONFIG_ENABLE_SENSOR_ACS712
        case SENSOR_TYPE_ACS712:
            ret = acs712_read_current(value);
            break;
#endif
        default:
            ret = ESP_ERR_NOT_FOUND;
            break;
    }
    
    if (ret == ESP_OK) {
        *value = *value * sensor_configs[type].calibration_scale + 
                 sensor_configs[type].calibration_offset;
    }
    
    return ret;
}

/**
 * @brief Read BMP280 (pressure/altitude)
 */
esp_err_t sensor_manager_read_pressure(float *pressure_hpa, float *altitude_m)
{
#ifdef CONFIG_ENABLE_SENSOR_BMP280
    return bmp280_read_pressure(pressure_hpa);
#else
    return ESP_ERR_NOT_FOUND;
#endif
}

/**
 * @brief Read SGP30 (CO2/TVOC)
 */
esp_err_t sensor_manager_read_air_quality(float *co2_ppm, float *tvoc_ppb)
{
#ifdef CONFIG_ENABLE_SENSOR_SGP30
    uint16_t co2, tvoc;
    esp_err_t ret = sgp30_read(&co2, &tvoc);
    
    if (ret == ESP_OK) {
        if (co2_ppm) *co2_ppm = (float)co2;
        if (tvoc_ppb) *tvoc_ppb = (float)tvoc;
    }
    
    return ret;
#else
    return ESP_ERR_NOT_FOUND;
#endif
}

/**
 * @brief Read PMS5003 (particulate matter)
 */
esp_err_t sensor_manager_read_particulate(uint16_t *pm25, uint16_t *pm10)
{
#ifdef CONFIG_ENABLE_SENSOR_PMS5003
    pms5003_data_t data;
    esp_err_t ret = pms5003_read(&data);
    
    if (ret == ESP_OK) {
        if (pm25) *pm25 = data.pm2_5_atm;
        if (pm10) *pm10 = data.pm10_atm;
    }
    
    return ret;
#else
    return ESP_ERR_NOT_FOUND;
#endif
}

/**
 * @brief Read HX711 (weight)
 */
esp_err_t sensor_manager_read_weight(float *weight_g)
{
#ifdef CONFIG_ENABLE_SENSOR_HX711
    return hx711_read_weight(weight_g);
#else
    return ESP_ERR_NOT_FOUND;
#endif
}

/**
 * @brief Read ACS712 (current/power)
 */
esp_err_t sensor_manager_read_current(float *current_amps, float *power_watts)
{
#ifdef CONFIG_ENABLE_SENSOR_ACS712
    esp_err_t ret = acs712_read_current(current_amps);
    
    if (ret == ESP_OK && power_watts) {
        *power_watts = *current_amps * 220.0f;
    }
    
    return ret;
#else
    return ESP_ERR_NOT_FOUND;
#endif
}

/**
 * @brief Tare (zero) weight sensor
 */
esp_err_t sensor_manager_tare_weight(uint8_t samples)
{
#ifdef CONFIG_ENABLE_SENSOR_HX711
    return hx711_tare(samples);
#else
    return ESP_ERR_NOT_FOUND;
#endif
}

/**
 * @brief Calibrate weight sensor with known weight
 */
esp_err_t sensor_manager_calibrate_weight(float known_weight_g)
{
#ifdef CONFIG_ENABLE_SENSOR_HX711
    int32_t raw;
    esp_err_t ret = hx711_read_raw(&raw);
    if (ret != ESP_OK) {
        return ret;
    }
    
    float cal_factor = (float)(raw - hx711_get_offset()) / known_weight_g;
    return hx711_set_calibration(cal_factor);
#else
    return ESP_ERR_NOT_FOUND;
#endif
}

/**
 * @brief Calibrate current sensor
 */
esp_err_t sensor_manager_calibrate_current(int offset)
{
#ifdef CONFIG_ENABLE_SENSOR_ACS712
    return acs712_set_offset(offset);
#else
    return ESP_ERR_NOT_FOUND;
#endif
}

/**
 * @brief Get cumulative energy
 */
esp_err_t sensor_manager_get_energy(float *energy_wh)
{
    if (energy_wh == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
#ifdef CONFIG_ENABLE_SENSOR_ACS712
    acs712_update_energy();
    *energy_wh = acs712_get_energy_wh();
    return ESP_OK;
#else
    return ESP_ERR_NOT_FOUND;
#endif
}

/**
 * @brief Reset energy counter
 */
esp_err_t sensor_manager_reset_energy(void)
{
#ifdef CONFIG_ENABLE_SENSOR_ACS712
    return acs712_reset_energy();
#else
    return ESP_ERR_NOT_FOUND;
#endif
}

/**
 * @brief Calibrate sensor
 */
esp_err_t sensor_manager_calibrate(sensor_type_t type, float reference)
{
    if (type >= SENSOR_TYPE_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!readings_valid[type]) {
        return ESP_FAIL;
    }
    
    sensor_configs[type].calibration_offset = reference - last_readings[type];
    
    ESP_LOGI(TAG, "Calibrated sensor %d: offset=%.2f", type, 
             sensor_configs[type].calibration_offset);
    
    return ESP_OK;
}

/**
 * @brief Get sensor configuration
 */
esp_err_t sensor_manager_get_config(sensor_type_t type, sensor_config_t *config)
{
    if (type >= SENSOR_TYPE_MAX || config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(config, &sensor_configs[type], sizeof(sensor_config_t));
    
    return ESP_OK;
}

/**
 * @brief Set sensor configuration
 */
esp_err_t sensor_manager_set_config(sensor_type_t type, const sensor_config_t *config)
{
    if (type >= SENSOR_TYPE_MAX || config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&sensor_configs[type], config, sizeof(sensor_config_t));
    
    return ESP_OK;
}

/**
 * @brief Enable/disable sensor
 */
esp_err_t sensor_manager_enable(sensor_type_t type, bool enable)
{
    if (type >= SENSOR_TYPE_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    
    sensor_configs[type].enabled = enable;
    
    return ESP_OK;
}

/**
 * @brief Self-test sensors
 */
esp_err_t sensor_manager_self_test(void)
{
    esp_err_t ret = ESP_OK;
    float value;
    
    ESP_LOGI(TAG, "Running sensor self-test...");
    
    for (int i = 0; i < SENSOR_TYPE_MAX; i++) {
        if (!sensor_configs[i].enabled) {
            continue;
        }
        
        ret = sensor_manager_read(i, &value);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Sensor %d: OK (value=%.2f)", i, value);
        } else {
            ESP_LOGW(TAG, "Sensor %d: FAILED (%s)", i, esp_err_to_name(ret));
        }
    }
    
    return ESP_OK;
}
