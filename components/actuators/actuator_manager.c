/**
 * Actuator Manager - Implementation
 */

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>

#include "actuator_manager.h"
#include "exhaust_fan.h"
#include "cooling_pad.h"
#include "heater.h"
#include "led_light.h"
#include "feeder.h"
#include "water_pump.h"
#include "alarm.h"

static const char *TAG = "ACTUATOR_MGR";

/* Global actuator state */
static actuator_state_t s_actuator_state = {
    .fan1_speed = 0,
    .fan2_speed = 0,
    .cooling_pad_on = false,
    .heater_level = 0,
    .light_level = 0,
    .feeder_active = false,
    .pump_on = false,
    .alarm_active = false
};

static bool s_initialized = false;
static bool s_emergency = false;

/**
 * @brief Initialize all actuators
 */
esp_err_t actuator_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing actuators...");
    
    /* Initialize exhaust fans */
    ESP_ERROR_CHECK(exhaust_fan_init());
    
    /* Initialize cooling pad */
    ESP_ERROR_CHECK(cooling_pad_init());
    
    /* Initialize heater */
    ESP_ERROR_CHECK(heater_init());
    
    /* Initialize LED light */
    ESP_ERROR_CHECK(led_light_init());
    
    /* Initialize feeder */
    ESP_ERROR_CHECK(feeder_init());
    
    /* Initialize water pump */
    ESP_ERROR_CHECK(water_pump_init());
    
    /* Initialize alarm */
    ESP_ERROR_CHECK(alarm_init());
    
    s_initialized = true;
    ESP_LOGI(TAG, "Actuator initialization complete");
    
    return ESP_OK;
}

/**
 * @brief Set actuator state
 */
esp_err_t actuator_manager_set(actuator_type_t type, uint8_t value)
{
    if (!s_initialized || s_emergency) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = ESP_OK;
    
    switch (type) {
        case ACTUATOR_TYPE_FAN1:
            ret = exhaust_fan_set_speed(0, value);
            s_actuator_state.fan1_speed = value;
            break;
        case ACTUATOR_TYPE_FAN2:
            ret = exhaust_fan_set_speed(1, value);
            s_actuator_state.fan2_speed = value;
            break;
        case ACTUATOR_TYPE_COOLING_PAD:
            ret = cooling_pad_set(value > 0);
            s_actuator_state.cooling_pad_on = (value > 0);
            break;
        case ACTUATOR_TYPE_HEATER:
            ret = heater_set_level(value);
            s_actuator_state.heater_level = value;
            break;
        case ACTUATOR_TYPE_LED_LIGHT:
            ret = led_light_set_level(value);
            s_actuator_state.light_level = value;
            break;
        case ACTUATOR_TYPE_FEEDER:
            if (value > 0) {
                ret = feeder_activate();
                s_actuator_state.feeder_active = true;
            }
            break;
        case ACTUATOR_TYPE_WATER_PUMP:
            ret = water_pump_set(value > 0);
            s_actuator_state.pump_on = (value > 0);
            break;
        case ACTUATOR_TYPE_BUZZER:
            ret = alarm_set(value > 0);
            s_actuator_state.alarm_active = (value > 0);
            break;
        default:
            return ESP_ERR_NOT_FOUND;
    }
    
    return ret;
}

/**
 * @brief Get actuator state
 */
esp_err_t actuator_manager_get(actuator_type_t type, uint8_t *value)
{
    if (value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    switch (type) {
        case ACTUATOR_TYPE_FAN1:
            *value = s_actuator_state.fan1_speed;
            break;
        case ACTUATOR_TYPE_FAN2:
            *value = s_actuator_state.fan2_speed;
            break;
        case ACTUATOR_TYPE_COOLING_PAD:
            *value = s_actuator_state.cooling_pad_on ? 1 : 0;
            break;
        case ACTUATOR_TYPE_HEATER:
            *value = s_actuator_state.heater_level;
            break;
        case ACTUATOR_TYPE_LED_LIGHT:
            *value = s_actuator_state.light_level;
            break;
        case ACTUATOR_TYPE_FEEDER:
            *value = s_actuator_state.feeder_active ? 1 : 0;
            break;
        case ACTUATOR_TYPE_WATER_PUMP:
            *value = s_actuator_state.pump_on ? 1 : 0;
            break;
        case ACTUATOR_TYPE_BUZZER:
            *value = s_actuator_state.alarm_active ? 1 : 0;
            break;
        default:
            return ESP_ERR_NOT_FOUND;
    }
    
    return ESP_OK;
}

/**
 * @brief Get all actuator states
 */
actuator_state_t* actuator_manager_get_state(void)
{
    return &s_actuator_state;
}

/**
 * @brief Emergency stop all
 */
esp_err_t actuator_manager_emergency_stop(void)
{
    ESP_LOGW(TAG, "EMERGENCY STOP ACTIVATED");
    s_emergency = true;
    
    /* Turn off all actuators */
    exhaust_fan_set_speed(0, 0);
    exhaust_fan_set_speed(1, 0);
    cooling_pad_set(false);
    heater_set_level(0);
    led_light_set_level(0);
    water_pump_set(false);
    alarm_set(true);  /* Keep alarm on */
    
    /* Update state */
    memset(&s_actuator_state, 0, sizeof(actuator_state_t));
    s_actuator_state.alarm_active = true;
    
    return ESP_OK;
}

/**
 * @brief Set all actuators to safe state
 */
esp_err_t actuator_manager_set_safe_state(void)
{
    ESP_LOGI(TAG, "Setting safe state");
    
    /* Turn off all actuators */
    exhaust_fan_set_speed(0, 0);
    exhaust_fan_set_speed(1, 0);
    cooling_pad_set(false);
    heater_set_level(0);
    led_light_set_level(50);  /* Default dim light */
    water_pump_set(false);
    alarm_set(false);
    
    /* Update state */
    memset(&s_actuator_state, 0, sizeof(actuator_state_t));
    s_actuator_state.light_level = 50;
    
    s_emergency = false;
    
    return ESP_OK;
}
