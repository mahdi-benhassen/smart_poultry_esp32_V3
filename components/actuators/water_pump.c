/**
 * Water Pump Driver - Implementation
 */

#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>

#include "water_pump.h"

static const char *TAG = "WATER_PUMP";

static bool s_on = false;
static bool s_initialized = false;

/**
 * @brief Initialize water pump
 */
esp_err_t water_pump_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_WATER_PUMP),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(GPIO_WATER_PUMP, 0);
    
    s_initialized = true;
    ESP_LOGI(TAG, "Water pump initialized on GPIO%d", GPIO_WATER_PUMP);
    
    return ESP_OK;
}

/**
 * @brief Set pump on/off
 */
esp_err_t water_pump_set(bool on)
{
    if (!s_initialized) return ESP_ERR_INVALID_STATE;
    
    gpio_set_level(GPIO_WATER_PUMP, on ? 1 : 0);
    s_on = on;
    
    ESP_LOGI(TAG, "Water pump: %s", on ? "ON" : "OFF");
    
    return ESP_OK;
}

/**
 * @brief Get pump state
 */
esp_err_t water_pump_get(bool *on)
{
    if (on == NULL) return ESP_ERR_INVALID_ARG;
    *on = s_on;
    return ESP_OK;
}
