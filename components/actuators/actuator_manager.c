/**
 * Actuator Manager - Implementation
 * Modular architecture - actuators can be enabled/disabled via Kconfig
 */

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>

#include "actuator_manager.h"

#ifdef CONFIG_ENABLE_ACTUATOR_EXHAUST_FAN
#include "exhaust_fan.h"
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_COOLING_PAD
#include "cooling_pad.h"
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_HEATER
#include "heater.h"
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_LED_LIGHT
#include "led_light.h"
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_FEEDER
#include "feeder.h"
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_WATER_PUMP
#include "water_pump.h"
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_ALARM
#include "alarm.h"
#endif

static const char *TAG = "ACTUATOR_MGR";

/* Global actuator state */
static actuator_state_t s_actuator_state = {
#ifdef CONFIG_ENABLE_ACTUATOR_EXHAUST_FAN
    .fan1_speed = 0,
    .fan2_speed = 0,
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_COOLING_PAD
    .cooling_pad_on = false,
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_HEATER
    .heater_level = 0,
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_LED_LIGHT
    .light_level = 0,
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_FEEDER
    .feeder_active = false,
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_WATER_PUMP
    .pump_on = false,
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_ALARM
    .alarm_active = false
#endif
};

static bool s_initialized = false;
static bool s_emergency = false;

/**
 * @brief Initialize all actuators
 */
esp_err_t actuator_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing actuators (Modular)...");
    
#ifdef CONFIG_ENABLE_ACTUATOR_EXHAUST_FAN
    ESP_ERROR_CHECK(exhaust_fan_init());
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_COOLING_PAD
    ESP_ERROR_CHECK(cooling_pad_init());
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_HEATER
    ESP_ERROR_CHECK(heater_init());
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_LED_LIGHT
    ESP_ERROR_CHECK(led_light_init());
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_FEEDER
    ESP_ERROR_CHECK(feeder_init());
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_WATER_PUMP
    ESP_ERROR_CHECK(water_pump_init());
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_ALARM
    ESP_ERROR_CHECK(alarm_init());
#endif

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
#ifdef CONFIG_ENABLE_ACTUATOR_EXHAUST_FAN
        case ACTUATOR_TYPE_FAN1:
            ret = exhaust_fan_set_speed(0, value);
            s_actuator_state.fan1_speed = value;
            break;
        case ACTUATOR_TYPE_FAN2:
            ret = exhaust_fan_set_speed(1, value);
            s_actuator_state.fan2_speed = value;
            break;
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_COOLING_PAD
        case ACTUATOR_TYPE_COOLING_PAD:
            ret = cooling_pad_set(value > 0);
            s_actuator_state.cooling_pad_on = (value > 0);
            break;
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_HEATER
        case ACTUATOR_TYPE_HEATER:
            ret = heater_set_level(value);
            s_actuator_state.heater_level = value;
            break;
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_LED_LIGHT
        case ACTUATOR_TYPE_LED_LIGHT:
            ret = led_light_set_level(value);
            s_actuator_state.light_level = value;
            break;
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_FEEDER
        case ACTUATOR_TYPE_FEEDER:
            if (value > 0) {
                ret = feeder_activate();
                s_actuator_state.feeder_active = true;
            }
            break;
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_WATER_PUMP
        case ACTUATOR_TYPE_WATER_PUMP:
            ret = water_pump_set(value > 0);
            s_actuator_state.pump_on = (value > 0);
            break;
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_ALARM
        case ACTUATOR_TYPE_BUZZER:
            ret = alarm_set(value > 0);
            s_actuator_state.alarm_active = (value > 0);
            break;
#endif
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
#ifdef CONFIG_ENABLE_ACTUATOR_EXHAUST_FAN
        case ACTUATOR_TYPE_FAN1:
            *value = s_actuator_state.fan1_speed;
            break;
        case ACTUATOR_TYPE_FAN2:
            *value = s_actuator_state.fan2_speed;
            break;
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_COOLING_PAD
        case ACTUATOR_TYPE_COOLING_PAD:
            *value = s_actuator_state.cooling_pad_on ? 1 : 0;
            break;
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_HEATER
        case ACTUATOR_TYPE_HEATER:
            *value = s_actuator_state.heater_level;
            break;
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_LED_LIGHT
        case ACTUATOR_TYPE_LED_LIGHT:
            *value = s_actuator_state.light_level;
            break;
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_FEEDER
        case ACTUATOR_TYPE_FEEDER:
            *value = s_actuator_state.feeder_active ? 1 : 0;
            break;
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_WATER_PUMP
        case ACTUATOR_TYPE_WATER_PUMP:
            *value = s_actuator_state.pump_on ? 1 : 0;
            break;
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_ALARM
        case ACTUATOR_TYPE_BUZZER:
            *value = s_actuator_state.alarm_active ? 1 : 0;
            break;
#endif
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
    
#ifdef CONFIG_ENABLE_ACTUATOR_EXHAUST_FAN
    exhaust_fan_set_speed(0, 0);
    exhaust_fan_set_speed(1, 0);
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_COOLING_PAD
    cooling_pad_set(false);
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_HEATER
    heater_set_level(0);
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_LED_LIGHT
    led_light_set_level(0);
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_WATER_PUMP
    water_pump_set(false);
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_ALARM
    alarm_set(true);
#endif
    
    memset(&s_actuator_state, 0, sizeof(actuator_state_t));
#ifdef CONFIG_ENABLE_ACTUATOR_ALARM
    s_actuator_state.alarm_active = true;
#endif
    
    return ESP_OK;
}

/**
 * @brief Set all actuators to safe state
 */
esp_err_t actuator_manager_set_safe_state(void)
{
    ESP_LOGI(TAG, "Setting safe state");
    
#ifdef CONFIG_ENABLE_ACTUATOR_EXHAUST_FAN
    exhaust_fan_set_speed(0, 0);
    exhaust_fan_set_speed(1, 0);
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_COOLING_PAD
    cooling_pad_set(false);
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_HEATER
    heater_set_level(0);
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_LED_LIGHT
    led_light_set_level(50);
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_WATER_PUMP
    water_pump_set(false);
#endif
#ifdef CONFIG_ENABLE_ACTUATOR_ALARM
    alarm_set(false);
#endif
    
    memset(&s_actuator_state, 0, sizeof(actuator_state_t));
#ifdef CONFIG_ENABLE_ACTUATOR_LED_LIGHT
    s_actuator_state.light_level = 50;
#endif
    
    s_emergency = false;
    
    return ESP_OK;
}
