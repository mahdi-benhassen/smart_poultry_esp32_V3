/**
 * Sensor Manager - Implementation
 */

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "sensor_manager.h"
#include "dht22.h"
#include "ds18b20.h"
#include "mq135.h"
#include "mq7.h"
#include "mq2.h"
#include "ldr.h"
#include "water_level.h"
#include "sound_sensor.h"

static const char *TAG = "SENSOR_MGR";

/* ADC Handle */
static adc_oneshot_unit_handle_t s_adc_handle = NULL;

/* Sensor configurations */
static sensor_config_t sensor_configs[SENSOR_TYPE_MAX] = {
    [SENSOR_TYPE_DHT22] = {
        .gpio = GPIO_DHT22,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
    [SENSOR_TYPE_DS18B20] = {
        .gpio = GPIO_DS18B20,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
    [SENSOR_TYPE_MQ135] = {
        .gpio = GPIO_MQ135,
        .adc_channel = ADC_MQ135,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
    [SENSOR_TYPE_MQ7] = {
        .gpio = GPIO_MQ7,
        .adc_channel = ADC_MQ7,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
    [SENSOR_TYPE_MQ2] = {
        .gpio = GPIO_MQ2,
        .adc_channel = ADC_MQ2,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
    [SENSOR_TYPE_LDR] = {
        .gpio = GPIO_LDR,
        .adc_channel = ADC_LDR,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
    [SENSOR_TYPE_WATER_LEVEL] = {
        .gpio = GPIO_WATER_LEVEL,
        .adc_channel = ADC_WATER_LEVEL,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    },
    [SENSOR_TYPE_SOUND] = {
        .gpio = GPIO_SOUND,
        .adc_channel = ADC_SOUND,
        .calibration_offset = 0.0f,
        .calibration_scale = 1.0f,
        .enabled = true
    }
};

/* Last sensor readings */
static float last_readings[SENSOR_TYPE_MAX] = {0};
static uint32_t last_timestamps[SENSOR_TYPE_MAX] = {0};
static bool readings_valid[SENSOR_TYPE_MAX] = {false};

/**
 * @brief Initialize all sensors
 */
esp_err_t sensor_manager_init(void)
{
    esp_err_t ret = ESP_OK;
    
    ESP_LOGI(TAG, "Initializing sensors...");
    
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
    
    /* MQ135 */
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_MQ135, &config));
    /* MQ7 */
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_MQ7, &config));
    /* MQ2 */
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_MQ2, &config));
    /* LDR */
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_LDR, &config));
    /* Water Level */
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_WATER_LEVEL, &config));
    /* Sound */
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_SOUND, &config));
    
    /* Initialize DHT22 */
    ret = dht22_init(sensor_configs[SENSOR_TYPE_DHT22].gpio);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize DHT22: %s", esp_err_to_name(ret));
    }
    
    /* Initialize DS18B20 */
    ret = ds18b20_init(sensor_configs[SENSOR_TYPE_DS18B20].gpio);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize DS18B20: %s", esp_err_to_name(ret));
    }
    
    /* Initialize MQ135 */
    ret = mq135_init(s_adc_handle, sensor_configs[SENSOR_TYPE_MQ135].adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MQ135: %s", esp_err_to_name(ret));
    }
    
    /* Initialize MQ7 */
    ret = mq7_init(s_adc_handle, sensor_configs[SENSOR_TYPE_MQ7].adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MQ7: %s", esp_err_to_name(ret));
    }
    
    /* Initialize MQ2 */
    ret = mq2_init(s_adc_handle, sensor_configs[SENSOR_TYPE_MQ2].adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MQ2: %s", esp_err_to_name(ret));
    }
    
    /* Initialize LDR */
    ret = ldr_init(s_adc_handle, sensor_configs[SENSOR_TYPE_LDR].adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LDR: %s", esp_err_to_name(ret));
    }
    
    /* Initialize Water Level */
    ret = water_level_init(s_adc_handle, sensor_configs[SENSOR_TYPE_WATER_LEVEL].adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Water Level sensor: %s", esp_err_to_name(ret));
    }
    
    /* Initialize Sound */
    ret = sound_sensor_init(s_adc_handle, sensor_configs[SENSOR_TYPE_SOUND].adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Sound sensor: %s", esp_err_to_name(ret));
    }
    
    ESP_LOGI(TAG, "Sensor initialization complete");
    
    return ESP_OK;
}

/**
 * @brief Read all sensor data
 */
esp_err_t sensor_manager_read_all(void *data)
{
    sensor_data_t *sensor_data = (sensor_data_t *)data;
    esp_err_t ret;
    uint32_t timestamp = esp_log_timestamp();
    
    /* Initialize with default values */
    memset(sensor_data, 0, sizeof(sensor_data_t));
    sensor_data->timestamp = timestamp;
    
    /* Read temperature and humidity from DHT22 */
    ret = dht22_read(&sensor_data->temperature, &sensor_data->humidity);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_DHT22] = sensor_data->temperature;
        last_timestamps[SENSOR_TYPE_DHT22] = timestamp;
        readings_valid[SENSOR_TYPE_DHT22] = true;
    }
    
    /* Read temperature from DS18B20 */
    ret = ds18b20_read(&sensor_data->temperature_ds18b20);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_DS18B20] = sensor_data->temperature_ds18b20;
        last_timestamps[SENSOR_TYPE_DS18B20] = timestamp;
        readings_valid[SENSOR_TYPE_DS18B20] = true;
    }
    
    /* Temperature selection: prefer DHT22 if valid, otherwise use DS18B20 */
    if (readings_valid[SENSOR_TYPE_DHT22]) {
        /* DHT22 is valid, use it as primary temperature */
        sensor_data->temperature = sensor_data->temperature;
    } else if (readings_valid[SENSOR_TYPE_DS18B20]) {
        /* DHT22 not valid, use DS18B20 as fallback */
        sensor_data->temperature = sensor_data->temperature_ds18b20;
    }
    
    /* Read gas sensors */
    ret = mq135_read(&sensor_data->ammonia_ppm);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_MQ135] = sensor_data->ammonia_ppm;
        last_timestamps[SENSOR_TYPE_MQ135] = timestamp;
        readings_valid[SENSOR_TYPE_MQ135] = true;
    }
    
    ret = mq7_read(&sensor_data->co_ppm);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_MQ7] = sensor_data->co_ppm;
        last_timestamps[SENSOR_TYPE_MQ7] = timestamp;
        readings_valid[SENSOR_TYPE_MQ7] = true;
    }
    
    ret = mq2_read(&sensor_data->lpg_ppm);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_MQ2] = sensor_data->lpg_ppm;
        last_timestamps[SENSOR_TYPE_MQ2] = timestamp;
        readings_valid[SENSOR_TYPE_MQ2] = true;
    }
    
    /* Read light level */
    ret = ldr_read(&sensor_data->light_percent);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_LDR] = sensor_data->light_percent;
        last_timestamps[SENSOR_TYPE_LDR] = timestamp;
        readings_valid[SENSOR_TYPE_LDR] = true;
    }
    
    /* Read water level */
    ret = water_level_read(&sensor_data->water_level);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_WATER_LEVEL] = sensor_data->water_level;
        last_timestamps[SENSOR_TYPE_WATER_LEVEL] = timestamp;
        readings_valid[SENSOR_TYPE_WATER_LEVEL] = true;
    }
    
    /* Read sound level */
    ret = sound_sensor_read(&sensor_data->sound_level);
    if (ret == ESP_OK) {
        last_readings[SENSOR_TYPE_SOUND] = sensor_data->sound_level;
        last_timestamps[SENSOR_TYPE_SOUND] = timestamp;
        readings_valid[SENSOR_TYPE_SOUND] = true;
    }
    
    return ESP_OK;
}

/**
 * @brief Read single sensor
 */
esp_err_t sensor_manager_read(sensor_type_t type, float *value)
{
    esp_err_t ret = ESP_OK;
    
    if (type >= SENSOR_TYPE_MAX || value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    switch (type) {
        case SENSOR_TYPE_DHT22:
            ret = dht22_read(value, value + 1);
            *value = *(value + 1);
            break;
        case SENSOR_TYPE_DS18B20:
            ret = ds18b20_read(value);
            break;
        case SENSOR_TYPE_MQ135:
            ret = mq135_read(value);
            break;
        case SENSOR_TYPE_MQ7:
            ret = mq7_read(value);
            break;
        case SENSOR_TYPE_MQ2:
            ret = mq2_read(value);
            break;
        case SENSOR_TYPE_LDR:
            ret = ldr_read(value);
            break;
        case SENSOR_TYPE_WATER_LEVEL:
            ret = water_level_read((uint8_t *)value);
            *value = *(uint8_t *)value;
            break;
        case SENSOR_TYPE_SOUND:
            ret = sound_sensor_read(value);
            break;
        default:
            return ESP_ERR_NOT_FOUND;
    }
    
    if (ret == ESP_OK) {
        /* Apply calibration */
        *value = *value * sensor_configs[type].calibration_scale + 
                 sensor_configs[type].calibration_offset;
    }
    
    return ret;
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
    
    /* Calculate calibration offset */
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
    
    /* Test each sensor */
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
