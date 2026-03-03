/**
 * BMP280 Barometric Pressure Sensor Driver - Implementation
 * For ventilation optimization and weather monitoring
 */

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <driver/i2c.h>
#include "bmp280.h"

static const char *TAG = "BMP280";

/* BMP280 I2C Address */
#define BMP280_I2C_ADDR_0          0x76
#define BMP280_I2C_ADDR_1          0x77

/* BMP280 Registers */
#define BMP280_REG_CHIPID         0xD0
#define BMP280_REG_RESET           0xE0
#define BMP280_REG_STATUS          0xF3
#define BMP280_REG_CTRL_MEAS       0xF4
#define BMP280_REG_CONFIG          0xF5
#define BMP280_REG_DATA            0xF7

/* Calibration data registers */
#define BMP280_REG_DIG_T1         0x88
#define BMP280_REG_DIG_T2         0x8A
#define BMP280_REG_DIG_T3         0x8C
#define BMP280_REG_DIG_P1         0x8E
#define BMP280_REG_DIG_P2         0x90
#define BMP280_REG_DIG_P3         0x92
#define BMP280_REG_DIG_P4         0x94
#define BMP280_REG_DIG_P5         0x96
#define BMP280_REG_DIG_P6         0x98
#define BMP280_REG_DIG_P7         0x9A
#define BMP280_REG_DIG_P8         0x9C
#define BMP280_REG_DIG_P9         0x9E

/* Mode settings */
#define BMP280_MODE_SLEEP          0x00
#define BMP280_MODE_FORCED         0x01
#define BMP280_MODE_NORMAL         0x03

/* Oversampling settings */
#define BMP280_OS_SKIP            0x00
#define BMP280_OS_1               0x01
#define BMP280_OS_2               0x02
#define BMP280_OS_4               0x03
#define BMP280_OS_8               0x04
#define BMP280_OS_16             0x05

/* Filter settings */
#define BMP280_FILTER_OFF         0x00
#define BMP280_FILTER_2           0x01
#define BMP280_FILTER_4           0x02
#define BMP280_FILTER_8           0x03
#define BMP280_FILTER_16         0x04

/* Global state */
static i2c_port_t s_i2c_port = 0;
static uint8_t s_i2c_addr = BMP280_I2C_ADDR_0;
static bool s_initialized = false;
static float s_sea_level_pressure = 1013.25f;

/* Calibration data */
static uint16_t s_dig_T1 = 0;
static int16_t s_dig_T2 = 0;
static int16_t s_dig_T3 = 0;
static uint16_t s_dig_P1 = 0;
static int16_t s_dig_P2 = 0;
static int16_t s_dig_P3 = 0;
static int16_t s_dig_P4 = 0;
static int16_t s_dig_P5 = 0;
static int16_t s_dig_P6 = 0;
static int16_t s_dig_P7 = 0;
static int16_t s_dig_P8 = 0;
static int16_t s_dig_P9 = 0;

/* Compensation parameters */
static int32_t s_t_fine = 0;

/**
 * @brief Write register
 */
static esp_err_t bmp280_write_reg(uint8_t reg, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (s_i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(s_i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

/**
 * @brief Read register
 */
static esp_err_t bmp280_read_reg(uint8_t reg, uint8_t *data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (s_i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (s_i2c_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, data, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(s_i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

/**
 * @brief Read multiple registers
 */
static esp_err_t bmp280_read_regs(uint8_t reg, uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (s_i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (s_i2c_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(s_i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

/**
 * @brief Read calibration data
 */
static esp_err_t bmp280_read_calibration(void)
{
    uint8_t calib[24];
    esp_err_t ret = bmp280_read_regs(BMP280_REG_DIG_T1, calib, 24);
    if (ret != ESP_OK) {
        return ret;
    }
    
    s_dig_T1 = (uint16_t)((calib[1] << 8) | calib[0]);
    s_dig_T2 = (int16_t)((calib[3] << 8) | calib[2]);
    s_dig_T3 = (int16_t)((calib[5] << 8) | calib[4]);
    s_dig_P1 = (uint16_t)((calib[7] << 8) | calib[6]);
    s_dig_P2 = (int16_t)((calib[9] << 8) | calib[8]);
    s_dig_P3 = (int16_t)((calib[11] << 8) | calib[10]);
    s_dig_P4 = (int16_t)((calib[13] << 8) | calib[12]);
    s_dig_P5 = (int16_t)((calib[15] << 8) | calib[14]);
    s_dig_P6 = (int16_t)((calib[17] << 8) | calib[16]);
    s_dig_P7 = (int16_t)((calib[19] << 8) | calib[18]);
    s_dig_P8 = (int16_t)((calib[21] << 8) | calib[20]);
    s_dig_P9 = (int16_t)((calib[23] << 8) | calib[22]);
    
    ESP_LOGD(TAG, "Calibration data loaded");
    
    return ESP_OK;
}

/**
 * @brief Initialize BMP280 sensor
 */
esp_err_t bmp280_init(i2c_port_t i2c_port, uint8_t i2c_addr)
{
    s_i2c_port = i2c_port;
    s_i2c_addr = i2c_addr;
    
    /* Check chip ID */
    uint8_t chip_id;
    esp_err_t ret = bmp280_read_reg(BMP280_REG_CHIPID, &chip_id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read chip ID");
        return ret;
    }
    
    if (chip_id != 0x58 && chip_id != 0x57) {
        ESP_LOGW(TAG, "Unexpected chip ID: 0x%02x (expected 0x58 or 0x57)", chip_id);
    }
    
    /* Read calibration data */
    ret = bmp280_read_calibration();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read calibration data");
        return ret;
    }
    
    /* Configure sensor: Normal mode, pressure oversampling x16, temperature oversampling x2 */
    uint8_t ctrl_meas = (BMP280_OS_16 << 5) | (BMP280_OS_2 << 2) | BMP280_MODE_NORMAL;
    ret = bmp280_write_reg(BMP280_REG_CTRL_MEAS, ctrl_meas);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ctrl_meas");
        return ret;
    }
    
    /* Configure filter and standby time */
    uint8_t config = (BMP280_FILTER_16 << 2) | (0x05 << 0);  /* Filter 16, standby 1000ms */
    ret = bmp280_write_reg(BMP280_REG_CONFIG, config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure config");
        return ret;
    }
    
    s_initialized = true;
    ESP_LOGI(TAG, "BMP280 initialized on I2C%d at 0x%02x", i2c_port, i2c_addr);
    
    return ESP_OK;
}

/**
 * @brief Compensate temperature
 */
static float bmp280_compensate_temperature(int32_t adc_T)
{
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)s_dig_T1 << 1))) * ((int32_t)s_dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)s_dig_T1)) * ((adc_T >> 4) - ((int32_t)s_dig_T1))) >> 12) * ((int32_t)s_dig_T3)) >> 14;
    s_t_fine = var1 + var2;
    T = (s_t_fine * 5 + 128) >> 8;
    
    return (float)T / 100.0f;
}

/**
 * @brief Compensate pressure
 */
static float bmp280_compensate_pressure(int32_t adc_P)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)s_t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)s_dig_P6;
    var2 = var2 + ((var1 * (int64_t)s_dig_P5) << 17);
    var2 = var2 + (((int64_t)s_dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)s_dig_P3) >> 8) + ((var1 * (int64_t)s_dig_P2) << 12);
    var1 = ((((int64_t)1) << 47) + var1) * ((int64_t)s_dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0.0f;
    }
    
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = ((int64_t)s_dig_P9 * (p >> 13) * (p >> 13)) >> 25;
    var2 = ((int64_t)s_dig_P8 * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)s_dig_P7) << 4);
    
    return (float)p / 256.0f;
}

/**
 * @brief Read temperature and pressure
 */
esp_err_t bmp280_read(float *temperature, float *pressure_hpa)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t data[6];
    esp_err_t ret = bmp280_read_regs(BMP280_REG_DATA, data, 6);
    if (ret != ESP_OK) {
        return ret;
    }
    
    int32_t adc_P = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | ((data[2] >> 4) & 0x0F);
    int32_t adc_T = ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | ((data[5] >> 4) & 0x0F);
    
    if (temperature != NULL) {
        *temperature = bmp280_compensate_temperature(adc_T);
    }
    
    if (pressure_hpa != NULL) {
        *pressure_hpa = bmp280_compensate_pressure(adc_P) / 256.0f;
    }
    
    ESP_LOGD(TAG, "Temp=%.2f°C, Pressure=%.2fhPa", 
             temperature ? *temperature : 0, pressure_hpa ? *pressure_hpa : 0);
    
    return ESP_OK;
}

/**
 * @brief Read only pressure
 */
esp_err_t bmp280_read_pressure(float *pressure_hpa)
{
    return bmp280_read(NULL, pressure_hpa);
}

/**
 * @brief Read altitude based on sea level pressure
 */
esp_err_t bmp280_read_altitude(float sea_level_hpa, float *altitude_m)
{
    float pressure;
    esp_err_t ret = bmp280_read_pressure(&pressure);
    if (ret != ESP_OK) {
        return ret;
    }
    
    /* Using barometric formula */
    *altitude_m = 44330.0f * (1.0f - powf(pressure / sea_level_hpa, 0.1903f));
    
    ESP_LOGD(TAG, "Altitude: %.1fm (pressure: %.1fhPa, sea-level: %.1fhPa)", 
             *altitude_m, pressure, sea_level_hpa);
    
    return ESP_OK;
}

/**
 * @brief Set sea level pressure for altitude calculation
 */
esp_err_t bmp280_set_sea_level_pressure(float pressure_hpa)
{
    s_sea_level_pressure = pressure_hpa;
    ESP_LOGI(TAG, "Sea level pressure set to %.1fhPa", pressure_hpa);
    return ESP_OK;
}

/**
 * @brief Get sensor calibration data
 */
esp_err_t bmp280_get_calibration(uint16_t *dig_T1, int16_t *dig_T2, int16_t *dig_T3,
                                  uint16_t *dig_P1, int16_t *dig_P2, int16_t *dig_P3,
                                  int16_t *dig_P4, int16_t *dig_P5, int16_t *dig_P6,
                                  int16_t *dig_P7, int16_t *dig_P8, int16_t *dig_P9)
{
    if (dig_T1) *dig_T1 = s_dig_T1;
    if (dig_T2) *dig_T2 = s_dig_T2;
    if (dig_T3) *dig_T3 = s_dig_T3;
    if (dig_P1) *dig_P1 = s_dig_P1;
    if (dig_P2) *dig_P2 = s_dig_P2;
    if (dig_P3) *dig_P3 = s_dig_P3;
    if (dig_P4) *dig_P4 = s_dig_P4;
    if (dig_P5) *dig_P5 = s_dig_P5;
    if (dig_P6) *dig_P6 = s_dig_P6;
    if (dig_P7) *dig_P7 = s_dig_P7;
    if (dig_P8) *dig_P8 = s_dig_P8;
    if (dig_P9) *dig_P9 = s_dig_P9;
    
    return ESP_OK;
}
