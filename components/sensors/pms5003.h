/**
 * PMS5003 Particulate Matter Sensor Driver
 * Measures PM1.0, PM2.5, and PM10 concentrations
 * Critical for poultry respiratory health monitoring
 */

#ifndef PMS5003_H
#define PMS5003_H

#include <stdint.h>
#include <esp_err.h>
#include <driver/uart.h>

/**
 * @brief PMS5003 data structure
 */
typedef struct {
    uint16_t pm1_0_std;      /* PM1.0 concentration (standard) */
    uint16_t pm2_5_std;       /* PM2.5 concentration (standard) */
    uint16_t pm10_std;       /* PM10 concentration (standard) */
    uint16_t pm1_0_atm;      /* PM1.0 concentration (atmospheric) */
    uint16_t pm2_5_atm;      /* PM2.5 concentration (atmospheric) */
    uint16_t pm10_atm;       /* PM10 concentration (atmospheric) */
    uint16_t particles_0_3;  /* Particles >0.3um count */
    uint16_t particles_0_5;  /* Particles >0.5um count */
    uint16_t particles_1_0;  /* Particles >1.0um count */
    uint16_t particles_2_5;  /* Particles >2.5um count */
    uint16_t particles_5_0;  /* Particles >5.0um count */
    uint16_t particles_10_0; /* Particles >10.0um count */
    uint16_t checksum;
} pms5003_data_t;

/**
 * @brief Initialize PMS5003 sensor
 */
esp_err_t pms5003_init(uart_port_t uart_num, int tx_pin, int rx_pin);

/**
 * @brief Read particulate matter data
 */
esp_err_t pms5003_read(pms5003_data_t *data);

/**
 * @brief Read PM2.5 value (most commonly used)
 */
esp_err_t pms5003_read_pm25(uint16_t *pm25_atm);

/**
 * @brief Read PM10 value
 */
esp_err_t pms5003_read_pm10(uint16_t *pm10_atm);

/**
 * @brief Trigger passive mode read
 */
esp_err_t pms5003_trigger_read(void);

/**
 * @brief Enter sleep mode
 */
esp_err_t pms5003_sleep(void);

/**
 * @brief Wake up from sleep mode
 */
esp_err_t pms5003_wakeup(void);

/**
 * @brief Perform factory reset
 */
esp_err_t pms5003_reset(void);

/**
 * @brief Set working mode
 */
esp_err_t pms5003_set_mode(uint8_t mode);

#endif /* PMS5003_H */
