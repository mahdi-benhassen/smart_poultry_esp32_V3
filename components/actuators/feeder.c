/**
 * Automatic Feeder Driver - Implementation
 */

#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "feeder.h"
#include "actuator_definitions.h"

static const char *TAG = "FEEDER";

#define FEEDER_ACTIVATE_TIME_MS   5000   /* 5 seconds */

static bool s_active = false;
static bool s_initialized = false;

/**
 * @brief Initialize feeder
 */
esp_err_t feeder_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_FEEDER),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(GPIO_FEEDER, 0);
    
    s_initialized = true;
    ESP_LOGI(TAG, "Feeder initialized on GPIO%d", GPIO_FEEDER);
    
    return ESP_OK;
}

/**
 * @brief Activate feeder
 */
esp_err_t feeder_activate(void)
{
    if (!s_initialized) return ESP_ERR_INVALID_STATE;
    
    ESP_LOGI(TAG, "Feeder activated");
    gpio_set_level(GPIO_FEEDER, 1);
    s_active = true;
    
    vTaskDelay(pdMS_TO_TICKS(FEEDER_ACTIVATE_TIME_MS));
    
    gpio_set_level(GPIO_FEEDER, 0);
    s_active = false;
    ESP_LOGI(TAG, "Feeder deactivated");
    
    return ESP_OK;
}

/**
 * @brief Get feeder state
 */
esp_err_t feeder_get_state(bool *active)
{
    if (active == NULL) return ESP_ERR_INVALID_ARG;
    *active = s_active;
    return ESP_OK;
}
