/**
 * SGP30 Multi-Pixel Gas Sensor Driver - Implementation
 * Measures CO2 equivalent and TVOC (Total Volatile Organic Compounds)
 * Critical for poultry house air quality monitoring
 */

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <driver/i2c.h>
#include "sgp30.h"

static const char *TAG = "SGP30";

/* SGP30 I2C Address */
#define SGP30_I2C_ADDR             0x58

/* SGP30 Commands */
#define SGP30_CMD_INIT_AIR_QUALITY     0x2003
#define SGP30_CMD_MEASURE_AIR_QUALITY   0x2008
#define SGP30_CMD_GET_BASELINE         0x2015
#define SGP30_CMD_SET_BASELINE         0x2017
#define SGP30_CMD_SET_HUMIDITY         0x2061
#define SGP30_CMD_MEASURE_TEST         0x2032
#define SGP30_CMD_GET_FEATURE_SET      0x202F
#define SGP30_CMD_GET_UNIQUE_ID        0x3682
#define SGP30_CMD_READ_SERIAL_ID       0x3687

/* Timing */
#define SGP30_INIT_DELAY_MS           10
#define SGP30_MEASURE_DELAY_MS        12
#define SGP30_TEST_DELAY_MS           220

/* Global state */
static i2c_port_t s_i2c_port = 0;
static bool s_initialized = false;
static bool s_iaq_init = false;
static uint8_t s_feature_set_version = 0;
static uint64_t s_unique_id = 0;

/* Forward declarations */
static uint8_t sgp30_crc8(uint8_t *data, uint8_t len);

/**
 * @brief Write command with parameters
 */
static esp_err_t sgp30_write_command(uint16_t cmd, uint16_t *data, uint8_t data_len)
{
    uint8_t buf[2 + data_len * 2];
    buf[0] = (cmd >> 8) & 0xFF;
    buf[1] = cmd & 0xFF;
    
    for (int i = 0; i < data_len; i++) {
        buf[2 + i * 2] = (data[i] >> 8) & 0xFF;
        buf[2 + i * 2 + 1] = data[i] & 0xFF;
    }
    
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SGP30_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, buf, 2 + data_len * 2, true);
    i2c_master_stop(cmd_handle);
    
    esp_err_t ret = i2c_master_cmd_begin(s_i2c_port, cmd_handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd_handle);
    
    return ret;
}

/**
 * @brief Read command result
 */
static esp_err_t sgp30_read_command(uint16_t cmd, uint16_t *data, uint8_t data_len, uint16_t delay_ms)
{
    uint8_t cmd_buf[2] = {(cmd >> 8) & 0xFF, cmd & 0xFF};
    
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SGP30_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, cmd_buf, 2, true);
    i2c_master_stop(cmd_handle);
    
    esp_err_t ret = i2c_master_cmd_begin(s_i2c_port, cmd_handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd_handle);
    
    if (ret != ESP_OK) {
        return ret;
    }
    
    if (delay_ms > 0) {
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
    
    cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SGP30_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle, (uint8_t *)data, data_len * 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd_handle);
    
    ret = i2c_master_cmd_begin(s_i2c_port, cmd_handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd_handle);
    
    return ret;
}

/**
 * @brief Initialize SGP30 sensor
 */
esp_err_t sgp30_init(i2c_port_t i2c_port)
{
    s_i2c_port = i2c_port;
    
    /* Get feature set version */
    uint16_t feature_data[1];
    esp_err_t ret = sgp30_read_command(SGP30_CMD_GET_FEATURE_SET, feature_data, 1, 2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get feature set version");
        return ret;
    }
    
    s_feature_set_version = (feature_data[0] >> 8) & 0xFF;
    ESP_LOGI(TAG, "SGP30 Feature Set Version: 0x%02x", s_feature_set_version);
    
    if (s_feature_set_version != 0x22) {
        ESP_LOGW(TAG, "Expected feature set version 0x22, got 0x%02x", s_feature_set_version);
    }
    
    /* Get unique ID */
    uint16_t serial_id[3];
    ret = sgp30_read_command(SGP30_CMD_READ_SERIAL_ID, serial_id, 3, 2);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to get serial ID");
    } else {
        s_unique_id = ((uint64_t)serial_id[0] << 32) | ((uint32_t)serial_id[1] << 16) | serial_id[2];
        ESP_LOGI(TAG, "SGP30 Unique ID: 0x%016llx", s_unique_id);
    }
    
    /* Initialize air quality measurement */
    ret = sgp30_write_command(SGP30_CMD_INIT_AIR_QUALITY, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize air quality measurement");
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(SGP30_INIT_DELAY_MS));
    
    s_initialized = true;
    s_iaq_init = true;
    ESP_LOGI(TAG, "SGP30 initialized on I2C%d", i2c_port);
    
    return ESP_OK;
}

/**
 * @brief Trigger IAQ measurement
 */
esp_err_t sgp30_measure(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    return sgp30_write_command(SGP30_CMD_MEASURE_AIR_QUALITY, NULL, 0);
}

/**
 * @brief Read CO2 equivalent and TVOC
 */
esp_err_t sgp30_read(uint16_t *co2_eq_ppm, uint16_t *tvoc_ppb)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    /* Trigger measurement */
    esp_err_t ret = sgp30_measure();
    if (ret != ESP_OK) {
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(SGP30_MEASURE_DELAY_MS));
    
    /* Read results */
    uint16_t data[3];
    ret = sgp30_read_command(SGP30_CMD_MEASURE_AIR_QUALITY, data, 3, 0);
    if (ret != ESP_OK) {
        return ret;
    }
    
    /* Check CRC */
    uint8_t crc1 = sgp30_crc8((uint8_t *)&data[0], 2);
    uint8_t crc2 = sgp30_crc8((uint8_t *)&data[1], 2);
    
    if (crc1 != (data[0] >> 8) && crc2 != (data[1] >> 8)) {
        ESP_LOGW(TAG, "CRC check failed");
    }
    
    if (co2_eq_ppm) {
        *co2_eq_ppm = data[0];
    }
    
    if (tvoc_ppb) {
        *tvoc_ppb = data[1];
    }
    
    ESP_LOGD(TAG, "CO2eq: %d ppm, TVOC: %d ppb", data[0], data[1]);
    
    return ESP_OK;
}

/**
 * @brief CRC8 calculation for SGP30
 */
static uint8_t sgp30_crc8(uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF;
    
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}

/**
 * @brief Read only CO2 equivalent
 */
esp_err_t sgp30_read_co2(uint16_t *co2_eq_ppm)
{
    return sgp30_read(co2_eq_ppm, NULL);
}

/**
 * @brief Read only TVOC
 */
esp_err_t sgp30_read_tvoc(uint16_t *tvoc_ppb)
{
    return sgp30_read(NULL, tvoc_ppb);
}

/**
 * @brief Get baseline for IAQ accuracy
 */
esp_err_t sgp30_get_baseline(uint16_t *co2_baseline, uint16_t *tvoc_baseline)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint16_t data[2];
    esp_err_t ret = sgp30_read_command(SGP30_CMD_GET_BASELINE, data, 2, 10);
    if (ret != ESP_OK) {
        return ret;
    }
    
    if (co2_baseline) {
        *co2_baseline = data[0];
    }
    
    if (tvoc_baseline) {
        *tvoc_baseline = data[1];
    }
    
    ESP_LOGD(TAG, "Baseline: CO2=%d, TVOC=%d", data[0], data[1]);
    
    return ESP_OK;
}

/**
 * @brief Set baseline for IAQ accuracy
 */
esp_err_t sgp30_set_baseline(uint16_t co2_baseline, uint16_t tvoc_baseline)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint16_t data[2] = {co2_baseline, tvoc_baseline};
    esp_err_t ret = sgp30_write_command(SGP30_CMD_SET_BASELINE, data, 2);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Baseline set: CO2=%d, TVOC=%d", co2_baseline, tvoc_baseline);
    }
    
    return ret;
}

/**
 * @brief Set humidity for compensation
 */
esp_err_t sgp30_set_humidity(uint32_t absolute_humidity)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    /* Convert to fixed point format */
    uint16_t humidity_raw = (absolute_humidity * 256) / 1000;
    
    esp_err_t ret = sgp30_write_command(SGP30_CMD_SET_HUMIDITY, &humidity_raw, 1);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Humidity compensation set: %lu (raw: %d)", 
                 absolute_humidity, humidity_raw);
    }
    
    return ret;
}

/**
 * @brief Perform self-test
 */
esp_err_t sgp30_self_test(uint8_t *feature_set_version)
{
    uint16_t data[1];
    esp_err_t ret = sgp30_read_command(SGP30_CMD_MEASURE_TEST, data, 1, SGP30_TEST_DELAY_MS);
    
    if (ret == ESP_OK) {
        if (data[0] == 0xD400) {
            ESP_LOGI(TAG, "SGP30 self-test passed");
        } else {
            ESP_LOGW(TAG, "SGP30 self-test result: 0x%04x", data[0]);
        }
    }
    
    if (feature_set_version) {
        *feature_set_version = s_feature_set_version;
    }
    
    return ret;
}

/**
 * @brief Get feature set version
 */
esp_err_t sgp30_get_feature_set_version(uint8_t *version)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (version) {
        *version = s_feature_set_version;
    }
    
    return ESP_OK;
}

/**
 * @brief Get unique ID
 */
esp_err_t sgp30_get_unique_id(uint64_t *unique_id)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (unique_id) {
        *unique_id = s_unique_id;
    }
    
    return ESP_OK;
}
