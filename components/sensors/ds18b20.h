/**
 * DS18B20 Temperature Sensor Driver
 */

#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>
#include <esp_err.h>
#include <driver/gpio.h>

/* DS18B20 commands */
#define DS18B20_CMD_SEARCH_ROM      0xF0
#define DS18B20_CMD_READ_ROM        0x33
#define DS18B20_CMD_MATCH_ROM       0x55
#define DS18B20_CMD_SKIP_ROM        0xCC
#define DS18B20_CMD_CONVERT_T       0x44
#define DS18B20_CMD_READ_SCRATCH    0xBE
#define DS18B20_CMD_WRITE_SCRATCH   0x4E

/* DS18B20 resolution */
typedef enum {
    DS18B20_RES_9BIT  = 0,
    DS18B20_RES_10BIT = 1,
    DS18B20_RES_11BIT = 2,
    DS18B20_RES_12BIT = 3
} ds18b20_resolution_t;

/* DS18B20 device */
typedef struct {
    uint8_t rom[8];
    bool valid;
} ds18b20_device_t;

/**
 * @brief Initialize DS18B20 bus
 */
esp_err_t ds18b20_init(gpio_num_t gpio);

/**
 * @brief Search for devices
 */
esp_err_t ds18b20_search(ds18b20_device_t *devices, uint8_t *count, uint8_t max_devices);

/**
 * @brief Read temperature from single device
 */
esp_err_t ds18b20_read(float *temperature);

/**
 * @brief Read temperature from specific ROM
 */
esp_err_t ds18b20_read_device(const uint8_t *rom, float *temperature);

/**
 * @brief Set resolution
 */
esp_err_t ds18b20_set_resolution(const uint8_t *rom, ds18b20_resolution_t resolution);

/**
 * @brief Start conversion on all devices
 */
esp_err_t ds18b20_start_conversion(void);

#endif /* DS18B20_H */
