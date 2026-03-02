/**
 * Cooling Pad Driver - Implementation
 */

#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>

#include "actuator_definitions.h"
#include "cooling_pad.h"

static const char *TAG = "COOLING_PAD";

static gpio_num_t s_gpio = GPIO_COOLING_PAD;
static bool s_on = false;
static bool s_initialized = false;

/**
 * @brief Initialize cooling pad
 */
esp_err_t cooling_pad_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << s_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    gpio_config(&io_conf);
    gpio_set_level(s_gpio, 0);
    
    s_initialized = true;
    ESP_LOGI(TAG, "Cooling pad initialized on GPIO%d", s_gpio);
    
    return ESP_OK;
}

/**
 * @brief Set cooling pad on/off
 */
esp_err_t cooling_pad_set(bool on)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    gpio_set_level(s_gpio, on ? 1 : 0);
    s_on = on;
    
    ESP_LOGI(TAG, "Cooling pad: %s", on ? "ON" : "OFF");
    
    return ESP_OK;
}

/**
 * @brief Get cooling pad state
 */
esp_err_t cooling_pad_get(bool *on)
{
    if (on == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *on = s_on;
    return ESP_OK;
}
