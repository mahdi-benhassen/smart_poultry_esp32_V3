/**
 * PMS5003 Particulate Matter Sensor Driver - Implementation
 * Measures PM1.0, PM2.5, and PM10 concentrations
 * Critical for poultry respiratory health monitoring
 */

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <driver/uart.h>
#include "pms5003.h"

static const char *TAG = "PMS5003";

/* PMS5003 Constants */
#define PMS5003_FRAME_LENGTH    32
#define PMS5003_HEADER_HIGH     0x42
#define PMS50025_HEADER_LOW    0x4D

/* PMS5003 Commands */
#define PMS5003_CMD_WAKEUP      0xE1
#define PMS5003_CMD_SLEEP      0xE2
#define PMS5003_CMD_MODE       0xE3
#define PMS5003_CMD_RD         0xE2
#define PMS5003_CMD_RESET       0xE4

/* UART Buffer Size */
#define PMS5003_BUF_SIZE       128

/* Global state */
static uart_port_t s_uart_num = UART_NUM_1;
static bool s_initialized = false;

/**
 * @brief Calculate checksum
 */
static uint16_t pms5003_calc_checksum(uint8_t *data, uint8_t len)
{
    uint16_t sum = 0;
    for (int i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

/**
 * @brief Initialize PMS5003 sensor
 */
esp_err_t pms5003_init(uart_port_t uart_num, int tx_pin, int rx_pin)
{
    s_uart_num = uart_num;
    
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    esp_err_t ret = uart_param_config(uart_num, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UART parameters");
        return ret;
    }
    
    ret = uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins");
        return ret;
    }
    
    ret = uart_driver_install(uart_num, PMS5003_BUF_SIZE * 2, 0, 0, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver");
        return ret;
    }
    
    /* Flush buffers */
    uart_flush_input(uart_num);
    
    s_initialized = true;
    ESP_LOGI(TAG, "PMS5003 initialized on UART%d (TX=%d, RX=%d)", uart_num, tx_pin, rx_pin);
    
    return ESP_OK;
}

/**
 * @brief Read data from UART buffer
 */
static esp_err_t pms5003_read_frame(pms5003_data_t *data)
{
    uint8_t buf[PMS5003_FRAME_LENGTH];
    int len = uart_read_bytes(s_uart_num, buf, PMS5003_FRAME_LENGTH, pdMS_TO_TICKS(1000));
    
    if (len < PMS5003_FRAME_LENGTH) {
        /* Try to find frame start */
        for (int i = 0; i < len - 1; i++) {
            if (buf[i] == PMS5003_HEADER_HIGH && buf[i + 1] == PMS50025_HEADER_LOW) {
                /* Shift buffer */
                if (i > 0) {
                    memmove(buf, &buf[i], len - i);
                    len = uart_read_bytes(s_uart_num, &buf[len - i], PMS5003_FRAME_LENGTH - (len - i), pdMS_TO_TICKS(1000));
                    len = PMS5003_FRAME_LENGTH;
                }
                break;
            }
        }
        
        if (len < PMS5003_FRAME_LENGTH) {
            return ESP_ERR_TIMEOUT;
        }
    }
    
    /* Verify header */
    if (buf[0] != PMS5003_HEADER_HIGH || buf[1] != PMS50025_HEADER_LOW) {
        ESP_LOGW(TAG, "Invalid frame header: 0x%02x 0x%02x", buf[0], buf[1]);
        return ESP_ERR_INVALID_RESPONSE;
    }
    
    /* Verify checksum */
    uint16_t checksum = (buf[30] << 8) | buf[31];
    uint16_t calculated = pms5003_calc_checksum(buf, 30);
    
    if (checksum != calculated) {
        ESP_LOGW(TAG, "Checksum mismatch: 0x%04x vs 0x%04x", checksum, calculated);
        return ESP_ERR_INVALID_CRC;
    }
    
    /* Parse data */
    data->pm1_0_std   = (buf[4] << 8) | buf[5];
    data->pm2_5_std   = (buf[6] << 8) | buf[7];
    data->pm10_std    = (buf[8] << 8) | buf[9];
    data->pm1_0_atm   = (buf[10] << 8) | buf[11];
    data->pm2_5_atm   = (buf[12] << 8) | buf[13];
    data->pm10_atm   = (buf[14] << 8) | buf[15];
    data->particles_0_3  = (buf[16] << 8) | buf[17];
    data->particles_0_5  = (buf[18] << 8) | buf[19];
    data->particles_1_0  = (buf[20] << 8) | buf[21];
    data->particles_2_5  = (buf[22] << 8) | buf[23];
    data->particles_5_0  = (buf[24] << 8) | buf[25];
    data->particles_10_0 = (buf[26] << 8) | buf[27];
    data->checksum    = checksum;
    
    ESP_LOGD(TAG, "PM1.0=%d, PM2.5=%d, PM10=%d (atmospheric)", 
             data->pm1_0_atm, data->pm2_5_atm, data->pm10_atm);
    
    return ESP_OK;
}

/**
 * @brief Read particulate matter data
 */
esp_err_t pms5003_read(pms5003_data_t *data)
{
    if (!s_initialized || data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* Flush input buffer */
    uart_flush_input(s_uart_num);
    
    /* Send read command */
    const uint8_t cmd[7] = {0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71};
    int written = uart_write_bytes(s_uart_num, (const char *)cmd, 7);
    if (written != 7) {
        return ESP_FAIL;
    }
    
    /* Read response */
    return pms5003_read_frame(data);
}

/**
 * @brief Read PM2.5 value
 */
esp_err_t pms5003_read_pm25(uint16_t *pm25_atm)
{
    pms5003_data_t data;
    esp_err_t ret = pms5003_read(&data);
    
    if (ret == ESP_OK && pm25_atm) {
        *pm25_atm = data.pm2_5_atm;
    }
    
    return ret;
}

/**
 * @brief Read PM10 value
 */
esp_err_t pms5003_read_pm10(uint16_t *pm10_atm)
{
    pms5003_data_t data;
    esp_err_t ret = pms5003_read(&data);
    
    if (ret == ESP_OK && pm10_atm) {
        *pm10_atm = data.pm10_atm;
    }
    
    return ret;
}

/**
 * @brief Trigger passive mode read
 */
esp_err_t pms5003_trigger_read(void)
{
    const uint8_t cmd[7] = {0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71};
    return uart_write_bytes(s_uart_num, (const char *)cmd, 7) == 7 ? ESP_OK : ESP_FAIL;
}

/**
 * @brief Enter sleep mode
 */
esp_err_t pms5003_sleep(void)
{
    const uint8_t cmd[7] = {0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x74};
    
    int written = uart_write_bytes(s_uart_num, (const char *)cmd, 7);
    if (written != 7) {
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "PMS5003 entered sleep mode");
    return ESP_OK;
}

/**
 * @brief Wake up from sleep mode
 */
esp_err_t pms5003_wakeup(void)
{
    /* Send wakeup pulse */
    const uint8_t wakeup[7] = {0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x75};
    
    int written = uart_write_bytes(s_uart_num, (const char *)wakeup, 7);
    if (written != 7) {
        return ESP_FAIL;
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    ESP_LOGI(TAG, "PMS5003 woke up");
    return ESP_OK;
}

/**
 * @brief Perform factory reset
 */
esp_err_t pms5003_reset(void)
{
    const uint8_t cmd[7] = {0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x74};
    
    int written = uart_write_bytes(s_uart_num, (const char *)cmd, 7);
    if (written != 7) {
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "PMS5003 reset");
    return ESP_OK;
}

/**
 * @brief Set working mode
 */
esp_err_t pms5003_set_mode(uint8_t mode)
{
    /* Mode: 0=Passive, 1=Active */
    uint8_t cmd[7] = {0x42, 0x4D, 0xE3, 0x00, 0x01, mode, 0};
    cmd[6] = pms5003_calc_checksum((uint8_t *)cmd, 6);
    
    int written = uart_write_bytes(s_uart_num, (const char *)cmd, 7);
    if (written != 7) {
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "PMS5003 mode set to %s", mode ? "Active" : "Passive");
    return ESP_OK;
}
