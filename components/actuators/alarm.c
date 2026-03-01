/**
 * Alarm Driver - Implementation
 */

#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/ledc.h>

#include "alarm.h"

static const char *TAG = "ALARM";

static bool s_on = false;
static bool s_initialized = false;

/**
 * @brief Initialize alarm
 */
esp_err_t alarm_init(void)
{
    /* Buzzer PWM */
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_3,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);
    
    ledc_channel_config_t channel_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_4,
        .timer_sel = LEDC_TIMER_3,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = GPIO_BUZZER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_conf);
    
    /* LED indicator */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_LED_INDICATOR),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_4, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_4);
    gpio_set_level(GPIO_LED_INDICATOR, 0);
    
    s_initialized = true;
    ESP_LOGI(TAG, "Alarm initialized");
    
    return ESP_OK;
}

/**
 * @brief Set alarm on/off
 */
esp_err_t alarm_set(bool on)
{
    if (!s_initialized) return ESP_ERR_INVALID_STATE;
    
    if (on) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_4, 128);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_4);
        gpio_set_level(GPIO_LED_INDICATOR, 1);
    } else {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_4, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_4);
        gpio_set_level(GPIO_LED_INDICATOR, 0);
    }
    
    s_on = on;
    ESP_LOGI(TAG, "Alarm: %s", on ? "ON" : "OFF");
    
    return ESP_OK;
}

/**
 * @brief Get alarm state
 */
esp_err_t alarm_get(bool *on)
{
    if (on == NULL) return ESP_ERR_INVALID_ARG;
    *on = s_on;
    return ESP_OK;
}
