/**
 * Heater Driver - Implementation
 */

#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/ledc.h>

#include "heater.h"

static const char *TAG = "HEATER";

static gpio_num_t s_gpio_pwm = GPIO_HEATER_PWM;
static gpio_num_t s_gpio_en = GPIO_HEATER_EN;
static uint8_t s_level = 0;
static bool s_initialized = false;

/**
 * @brief Initialize heater
 */
esp_err_t heater_init(void)
{
    /* Enable GPIO */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << s_gpio_en),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    /* PWM GPIO */
    io_conf.pin_bit_mask = (1ULL << s_gpio_pwm);
    gpio_config(&io_conf);
    
    /* LEDC */
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_1,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);
    
    ledc_channel_config_t channel_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_2,
        .timer_sel = LEDC_TIMER_1,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = s_gpio_pwm,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_conf);
    
    gpio_set_level(s_gpio_en, 0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
    
    s_initialized = true;
    ESP_LOGI(TAG, "Heater initialized");
    
    return ESP_OK;
}

/**
 * @brief Set heater level
 */
esp_err_t heater_set_level(uint8_t level)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (level > 100) level = 100;
    
    uint32_t duty = (level * 1023) / 100;
    
    if (level > 0) {
        gpio_set_level(s_gpio_en, 1);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
    } else {
        gpio_set_level(s_gpio_en, 0);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
    }
    
    s_level = level;
    ESP_LOGD(TAG, "Heater level: %d%%", level);
    
    return ESP_OK;
}

/**
 * @brief Get heater level
 */
esp_err_t heater_get_level(uint8_t *level)
{
    if (level == NULL) return ESP_ERR_INVALID_ARG;
    *level = s_level;
    return ESP_OK;
}
