/**
 * Exhaust Fan Driver - Implementation
 */

#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/ledc.h>

#include "actuator_definitions.h"
#include "exhaust_fan.h"

static const char *TAG = "EXHAUST_FAN";

/* GPIO pins */
static const gpio_num_t FAN_PWM_GPIO[FAN_COUNT] = {GPIO_FAN1_PWM, GPIO_FAN2_PWM};
static const gpio_num_t FAN_EN_GPIO[FAN_COUNT] = {GPIO_FAN1_EN, GPIO_FAN2_EN};

/* LEDC configuration */
static const ledc_timer_t FAN_TIMER = LEDC_TIMER_0;
static const ledc_mode_t FAN_SPEED_MODE = LEDC_LOW_SPEED_MODE;
static const uint8_t FAN_CHANNEL[FAN_COUNT] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1};

/* State */
static uint8_t s_fan_speed[FAN_COUNT] = {0};
static bool s_initialized = false;

/**
 * @brief Initialize exhaust fans
 */
esp_err_t exhaust_fan_init(void)
{
    ESP_LOGI(TAG, "Initializing exhaust fans...");
    
    for (int i = 0; i < FAN_COUNT; i++) {
        /* Configure enable GPIO */
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << FAN_EN_GPIO[i]),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        gpio_config(&io_conf);
        
        /* Configure PWM GPIO */
        io_conf.pin_bit_mask = (1ULL << FAN_PWM_GPIO[i]);
        io_conf.mode = GPIO_MODE_OUTPUT;
        gpio_config(&io_conf);
        
        /* Setup LEDC timer */
        ledc_timer_config_t timer_conf = {
            .speed_mode = FAN_SPEED_MODE,
            .timer_num = FAN_TIMER,
            .duty_resolution = LEDC_TIMER_10_BIT,
            .freq_hz = 1000,
            .clk_cfg = LEDC_AUTO_CLK
        };
        
        if (i == 0) {
            ledc_timer_config(&timer_conf);
        }
        
        /* Setup LEDC channel */
        ledc_channel_config_t channel_conf = {
            .speed_mode = FAN_SPEED_MODE,
            .channel = FAN_CHANNEL[i],
            .timer_sel = FAN_TIMER,
            .intr_type = LEDC_INTR_DISABLE,
            .gpio_num = FAN_PWM_GPIO[i],
            .duty = 0,
            .hpoint = 0
        };
        ledc_channel_config(&channel_conf);
        
        /* Initialize fan */
        gpio_set_level(FAN_EN_GPIO[i], 0);
        ledc_set_duty(FAN_SPEED_MODE, FAN_CHANNEL[i], 0);
        ledc_update_duty(FAN_SPEED_MODE, FAN_CHANNEL[i]);
        
        s_fan_speed[i] = 0;
    }
    
    s_initialized = true;
    ESP_LOGI(TAG, "Exhaust fans initialized");
    
    return ESP_OK;
}

/**
 * @brief Set fan speed
 */
esp_err_t exhaust_fan_set_speed(uint8_t fan_id, uint8_t speed)
{
    if (fan_id >= FAN_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }
    
    /* Clamp speed to 0-100 */
    if (speed > 100) speed = 100;
    
    uint32_t duty = (speed * 1023) / 100;  /* Convert to 10-bit duty */
    
    if (speed > 0) {
        /* Enable fan and set PWM */
        gpio_set_level(FAN_EN_GPIO[fan_id], 1);
        ledc_set_duty(FAN_SPEED_MODE, FAN_CHANNEL[fan_id], duty);
        ledc_update_duty(FAN_SPEED_MODE, FAN_CHANNEL[fan_id]);
    } else {
        /* Disable fan */
        gpio_set_level(FAN_EN_GPIO[fan_id], 0);
        ledc_set_duty(FAN_SPEED_MODE, FAN_CHANNEL[fan_id], 0);
        ledc_update_duty(FAN_SPEED_MODE, FAN_CHANNEL[fan_id]);
    }
    
    s_fan_speed[fan_id] = speed;
    ESP_LOGD(TAG, "Fan %d speed: %d%%", fan_id, speed);
    
    return ESP_OK;
}

/**
 * @brief Get fan speed
 */
esp_err_t exhaust_fan_get_speed(uint8_t fan_id, uint8_t *speed)
{
    if (fan_id >= FAN_COUNT || speed == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *speed = s_fan_speed[fan_id];
    return ESP_OK;
}

/**
 * @brief Set all fans to specific speed
 */
esp_err_t exhaust_fan_set_all(uint8_t speed)
{
    for (int i = 0; i < FAN_COUNT; i++) {
        exhaust_fan_set_speed(i, speed);
    }
    return ESP_OK;
}
