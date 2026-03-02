/**
 * LED Light Driver - Implementation
 */

#include <esp_log.h>
#include <esp_err.h>
#include <driver/ledc.h>

#include "led_light.h"

#include "actuator_definitions.h"

static const char *TAG = "LED_LIGHT";

static uint8_t s_level = 0;
static bool s_initialized = false;

/**
 * @brief Initialize LED light
 */
esp_err_t led_light_init(void)
{
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_2,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);
    
    ledc_channel_config_t channel_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_3,
        .timer_sel = LEDC_TIMER_2,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = GPIO_LED_LIGHT,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_conf);
    
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3);
    
    s_initialized = true;
    ESP_LOGI(TAG, "LED light initialized on GPIO%d", GPIO_LED_LIGHT);
    
    return ESP_OK;
}

/**
 * @brief Set light level
 */
esp_err_t led_light_set_level(uint8_t level)
{
    if (!s_initialized) return ESP_ERR_INVALID_STATE;
    
    if (level > 100) level = 100;
    
    uint32_t duty = (level * 1023) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3);
    
    s_level = level;
    ESP_LOGD(TAG, "LED light: %d%%", level);
    
    return ESP_OK;
}

/**
 * @brief Get light level
 */
esp_err_t led_light_get_level(uint8_t *level)
{
    if (level == NULL) return ESP_ERR_INVALID_ARG;
    *level = s_level;
    return ESP_OK;
}
