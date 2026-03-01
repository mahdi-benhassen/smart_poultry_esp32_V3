/**
 * Actuator Manager - Header File
 */

#ifndef ACTUATOR_MANAGER_H
#define ACTUATOR_MANAGER_H

#include <stdint.h>
#include <esp_err.h>

/* Actuator types */
typedef enum {
    ACTUATOR_TYPE_FAN1,
    ACTUATOR_TYPE_FAN2,
    ACTUATOR_TYPE_COOLING_PAD,
    ACTUATOR_TYPE_HEATER,
    ACTUATOR_TYPE_LED_LIGHT,
    ACTUATOR_TYPE_FEEDER,
    ACTUATOR_TYPE_WATER_PUMP,
    ACTUATOR_TYPE_BUZZER,
    ACTUATOR_TYPE_LED_INDICATOR,
    ACTUATOR_TYPE_MAX
} actuator_type_t;

/* Actuator state */
typedef struct {
    uint8_t fan1_speed;
    uint8_t fan2_speed;
    bool cooling_pad_on;
    uint8_t heater_level;
    uint8_t light_level;
    bool feeder_active;
    bool pump_on;
    bool alarm_active;
} actuator_state_t;

/**
 * @brief Initialize all actuators
 */
esp_err_t actuator_manager_init(void);

/**
 * @brief Set actuator state
 */
esp_err_t actuator_manager_set(actuator_type_t type, uint8_t value);

/**
 * @brief Get actuator state
 */
esp_err_t actuator_manager_get(actuator_type_t type, uint8_t *value);

/**
 * @brief Get all actuator states
 */
actuator_state_t* actuator_manager_get_state(void);

/**
 * @brief Emergency stop all
 */
esp_err_t actuator_manager_emergency_stop(void);

/**
 * @brief Set all actuators to safe state
 */
esp_err_t actuator_manager_set_safe_state(void);

#endif /* ACTUATOR_MANAGER_H */
