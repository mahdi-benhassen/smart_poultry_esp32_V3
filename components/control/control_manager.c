/**
 * Control Manager - Implementation
 */

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "control_manager.h"
#include "pid_controller.h"
#include "rules_engine.h"
#include "decision_algorithm.h"
#include "emergency_protocol.h"
#include "energy_optimizer.h"
#include "actuator_manager.h"

static const char *TAG = "CONTROL_MGR";

/* Global sensor data */
static sensor_data_t s_sensor_data;
static bool s_initialized = false;
static SemaphoreHandle_t s_sensor_mutex = NULL;

/* Default thresholds */
static struct {
    float temp_min;
    float temp_max;
    float temp_optimal;
    float humidity_min;
    float humidity_max;
    float ammonia_max;
    float co_max;
} s_thresholds = {
    .temp_min = 18.0f,
    .temp_max = 32.0f,
    .temp_optimal = 25.0f,
    .humidity_min = 40.0f,
    .humidity_max = 80.0f,
    .ammonia_max = 25.0f,
    .co_max = 10.0f
};

/**
 * @brief Initialize control system
 */
esp_err_t control_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing control system...");
    
    /* Create mutex for sensor data protection */
    s_sensor_mutex = xSemaphoreCreateMutex();
    if (s_sensor_mutex == NULL) {
        return ESP_ERR_NO_MEM;
    }
    
    /* Initialize PID controllers */
    pid_controller_init();
    
    /* Initialize rules engine */
    rules_engine_init();
    
    /* Initialize decision algorithm */
    decision_algorithm_init();
    
    /* Initialize emergency protocol */
    emergency_protocol_init();
    
    /* Initialize energy optimizer */
    energy_optimizer_init();
    
    /* Set default thresholds */
    rules_engine_set_thresholds(&s_thresholds);
    
    s_initialized = true;
    ESP_LOGI(TAG, "Control system initialized");
    
    return ESP_OK;
}

/**
 * @brief Update control loop
 */
esp_err_t control_manager_update(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    /* Check for emergency conditions first */
    if (emergency_protocol_check(&s_sensor_data)) {
        ESP_LOGW(TAG, "Emergency protocol activated");
        return ESP_OK;
    }
    
    /* Run rules engine */
    rules_engine_result_t rules_result;
    rules_engine_evaluate(&s_sensor_data, &rules_result);
    
    /* Get decision from algorithm */
    decision_output_t decision;
    decision_algorithm_evaluate(&s_sensor_data, &decision);
    
    /* Apply energy optimization */
    energy_optimizer_optimize(&decision);
    
    /* Apply control outputs */
    actuator_manager_set(ACTUATOR_TYPE_FAN1, decision.fan1_speed);
    actuator_manager_set(ACTUATOR_TYPE_FAN2, decision.fan2_speed);
    actuator_manager_set(ACTUATOR_TYPE_COOLING_PAD, decision.cooling_pad_on ? 100 : 0);
    actuator_manager_set(ACTUATOR_TYPE_HEATER, decision.heater_level);
    actuator_manager_set(ACTUATOR_TYPE_LED_LIGHT, decision.light_level);
    
    /* Handle feeder */
    if (decision.feeder_activate) {
        actuator_manager_set(ACTUATOR_TYPE_FEEDER, 1);
    }
    
    /* Handle pump */
    actuator_manager_set(ACTUATOR_TYPE_WATER_PUMP, decision.pump_on ? 1 : 0);
    
    ESP_LOGD(TAG, "Control update: Fan1=%d, Fan2=%d, Cooling=%d, Heater=%d, Light=%d",
             decision.fan1_speed, decision.fan2_speed,
             decision.cooling_pad_on, decision.heater_level, decision.light_level);
    
    return ESP_OK;
}

/**
 * @brief Update sensor data
 */
esp_err_t control_manager_update_sensors(const sensor_data_t *data)
{
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_sensor_mutex != NULL) {
        xSemaphoreTake(s_sensor_mutex, portMAX_DELAY);
    }
    memcpy(&s_sensor_data, data, sizeof(sensor_data_t));
    if (s_sensor_mutex != NULL) {
        xSemaphoreGive(s_sensor_mutex);
    }
    
    return ESP_OK;
}

/**
 * @brief Get current sensor data
 */
sensor_data_t* control_manager_get_sensor_data(void)
{
    if (s_sensor_mutex != NULL) {
        xSemaphoreTake(s_sensor_mutex, portMAX_DELAY);
    }
    sensor_data_t *result = &s_sensor_data;
    /* Note: Caller must release mutex after use - simplified for now */
    return result;
}

/**
 * @brief Update configuration
 */
esp_err_t control_manager_set_config(const void *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&s_thresholds, config, sizeof(s_thresholds));
    rules_engine_set_thresholds(&s_thresholds);
    
    ESP_LOGI(TAG, "Configuration updated");
    
    return ESP_OK;
}

/**
 * @brief Get configuration
 */
esp_err_t control_manager_get_config(void *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(config, &s_thresholds, sizeof(s_thresholds));
    
    return ESP_OK;
}
