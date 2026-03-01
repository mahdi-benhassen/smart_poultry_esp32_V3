/**
 * DS18B20 Temperature Sensor Driver - Implementation
 */

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "ds18b20.h"

static const char *TAG = "DS18B20";

#define DS18B20_TIMEOUT_US      250
#define MAX_DEVICES             4

static gpio_num_t s_gpio = GPIO_DS18B20;
static bool s_initialized = false;
static ds18b20_device_t s_devices[MAX_DEVICES];
static uint8_t s_device_count = 0;

/* OneWire functions */
static void onewire_init(gpio_num_t gpio);
static void onewire_write_bit(gpio_num_t gpio, uint8_t bit);
static uint8_t onewire_read_bit(gpio_num_t gpio);
static void onewire_write_byte(gpio_num_t gpio, uint8_t byte);
static uint8_t onewire_read_byte(gpio_num_t gpio);
static uint8_t onewire_crc8(const uint8_t *data, uint8_t len);
static int onewire_search(gpio_num_t gpio, uint8_t *rom_codes, uint8_t max_devices);

/**
 * @brief Initialize DS18B20 bus
 */
esp_err_t ds18b20_init(gpio_num_t gpio)
{
    s_gpio = gpio;
    
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    
    /* Search for devices */
    s_device_count = 0;
    esp_err_t ret = ds18b20_search(s_devices, &s_device_count, MAX_DEVICES);
    
    if (ret == ESP_OK && s_device_count > 0) {
        ESP_LOGI(TAG, "Found %d DS18B20 device(s)", s_device_count);
        s_initialized = true;
    } else {
        ESP_LOGW(TAG, "No DS18B20 devices found");
    }
    
    return ESP_OK;
}

/**
 * @brief Search for devices
 */
esp_err_t ds18b20_search(ds18b20_device_t *devices, uint8_t *count, uint8_t max_devices)
{
    int found = onewire_search(s_gpio, (uint8_t *)devices, max_devices);
    *count = found > 0 ? found : 0;
    return ESP_OK;
}

/**
 * @brief Read temperature from single device
 */
esp_err_t ds18b20_read(float *temperature)
{
    if (!s_initialized || s_device_count == 0) {
        *temperature = -999.0f;
        return ESP_ERR_INVALID_STATE;
    }
    
    /* Read from first device */
    return ds18b20_read_device(s_devices[0].rom, temperature);
}

/**
 * @brief Read temperature from specific ROM
 */
esp_err_t ds18b20_read_device(const uint8_t *rom, float *temperature)
{
    if (rom == NULL || temperature == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t data[9];
    
    /* Send reset */
    gpio_set_direction(s_gpio, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(s_gpio, 0);
    ets_delay_us(480);
    gpio_set_level(s_gpio, 1);
    ets_delay_us(480);
    
    /* Match ROM */
    onewire_write_byte(s_gpio, DS18B20_CMD_MATCH_ROM);
    for (int i = 0; i < 8; i++) {
        onewire_write_byte(s_gpio, rom[i]);
    }
    
    /* Read scratchpad */
    onewire_write_byte(s_gpio, DS18B20_CMD_READ_SCRATCH);
    for (int i = 0; i < 9; i++) {
        data[i] = onewire_read_byte(s_gpio);
    }
    
    /* Verify CRC */
    if (onewire_crc8(data, 8) != data[8]) {
        ESP_LOGE(TAG, "CRC error reading DS18B20");
        return ESP_ERR_INVALID_CRC;
    }
    
    /* Convert to temperature */
    int16_t raw_temp = (data[1] << 8) | data[0];
    
    if (raw_temp & 0x8000) {
        raw_temp = ~raw_temp + 1;
        *temperature = -(float)raw_temp / 16.0f;
    } else {
        *temperature = (float)raw_temp / 16.0f;
    }
    
    ESP_LOGD(TAG, "DS18B20 temperature: %.2fC", *temperature);
    
    return ESP_OK;
}

/**
 * @brief Set resolution
 */
esp_err_t ds18b20_set_resolution(const uint8_t *rom, ds18b20_resolution_t resolution)
{
    if (rom == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t config = (resolution << 5) | 0x1F;
    
    /* Reset */
    gpio_set_direction(s_gpio, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(s_gpio, 0);
    ets_delay_us(480);
    gpio_set_level(s_gpio, 1);
    ets_delay_us(480);
    
    /* Match ROM and write scratchpad */
    onewire_write_byte(s_gpio, DS18B20_CMD_MATCH_ROM);
    for (int i = 0; i < 8; i++) {
        onewire_write_byte(s_gpio, rom[i]);
    }
    onewire_write_byte(s_gpio, DS18B20_CMD_WRITE_SCRATCH);
    onewire_write_byte(s_gpio, 0x00);  /* TH (not used) */
    onewire_write_byte(s_gpio, 0x00);  /* TL (not used) */
    onewire_write_byte(s_gpio, config); /* Configuration */
    
    return ESP_OK;
}

/**
 * @brief Start conversion on all devices
 */
esp_err_t ds18b20_start_conversion(void)
{
    /* Reset */
    gpio_set_direction(s_gpio, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(s_gpio, 0);
    ets_delay_us(480);
    gpio_set_level(s_gpio, 1);
    ets_delay_us(480);
    
    /* Skip ROM and start conversion */
    onewire_write_byte(s_gpio, DS18B20_CMD_SKIP_ROM);
    onewire_write_byte(s_gpio, DS18B20_CMD_CONVERT_T);
    
    /* Wait for conversion (12-bit = 750ms) */
    vTaskDelay(pdMS_TO_TICKS(750));
    
    return ESP_OK;
}

/* OneWire helper functions */
static void onewire_init(gpio_num_t gpio)
{
    gpio_set_direction(gpio, GPIO_MODE_INPUT);
}

static void onewire_write_bit(gpio_num_t gpio, uint8_t bit)
{
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(gpio, 0);
    
    if (bit) {
        ets_delay_us(10);
        gpio_set_level(gpio, 1);
        ets_delay_us(55);
    } else {
        ets_delay_us(65);
        gpio_set_level(gpio, 1);
        ets_delay_us(5);
    }
}

static uint8_t onewire_read_bit(gpio_num_t gpio)
{
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(gpio, 0);
    ets_delay_us(2);
    gpio_set_level(gpio, 1);
    gpio_set_direction(gpio, GPIO_MODE_INPUT);
    ets_delay_us(10);
    
    uint8_t bit = gpio_get_level(gpio);
    ets_delay_us(50);
    
    return bit;
}

static void onewire_write_byte(gpio_num_t gpio, uint8_t byte)
{
    for (int i = 0; i < 8; i++) {
        onewire_write_bit(gpio, byte & 0x01);
        byte >>= 1;
    }
}

static uint8_t onewire_read_byte(gpio_num_t gpio)
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte >>= 1;
        if (onewire_read_bit(gpio)) {
            byte |= 0x80;
        }
    }
    return byte;
}

static uint8_t onewire_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x01) {
                crc = (crc >> 1) ^ 0x8C;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

static int onewire_search(gpio_num_t gpio, uint8_t *rom_codes, uint8_t max_devices)
{
    int found = 0;
    uint8_t diff, rom[8];
    
    for (diff = DS18B20_CMD_SEARCH_ROM; diff != 0; ) {
        uint8_t cmd = DS18B20_CMD_SEARCH_ROM;
        
        /* Reset */
        gpio_set_direction(gpio, GPIO_MODE_OUTPUT_OD);
        gpio_set_level(gpio, 0);
        ets_delay_us(480);
        gpio_set_level(gpio, 1);
        ets_delay_us(480);
        
        /* Search ROM */
        uint8_t last_zero = 0;
        
        for (int i = 0; i < 64; i++) {
            int id_bit = onewire_read_bit(gpio);
            int cmp_id_bit = onewire_read_bit(gpio);
            
            if (id_bit && cmp_id_bit) {
                break;
            }
            
            int bit_number = i / 8;
            int bit_position = i % 8;
            
            if (!id_bit && !cmp_id_bit) {
                if (bit_position == last_zero) {
                    /* Write 0 - take path with no devices */
                    onewire_write_bit(gpio, 0);
                    rom[bit_number] &= ~(1 << bit_position);
                    last_zero = i;
                } else if (bit_position > last_zero) {
                    /* Write 1 - take path with devices */
                    onewire_write_bit(gpio, 1);
                    rom[bit_number] |= (1 << bit_position);
                } else {
                    if ((rom[bit_number] & (1 << bit_position)) == 0) {
                        last_zero = i;
                    }
                }
            } else if (id_bit) {
                onewire_write_bit(gpio, 1);
                rom[bit_number] |= (1 << bit_position);
            } else {
                onewire_write_bit(gpio, 0);
                rom[bit_number] &= ~(1 << bit_position);
            }
        }
        
        if (i == 64) {
            /* Found a device */
            memcpy(&rom_codes[found * 8], rom, 8);
            found++;
            if (found >= max_devices) {
                break;
            }
        } else {
            break;
        }
    }
    
    return found;
}
