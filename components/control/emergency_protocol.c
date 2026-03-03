/**
 * Emergency Protocol - Implementation
 * Tunisian Mediterranean Climate Poultry Standards
 */

#include <esp_log.h>
#include "emergency_protocol.h"
#include "actuator_manager.h"
#include "../../main/include/main.h"

static const char *TAG = "EMERGENCY";

static bool s_emergency_active = false;

/**
 * @brief Initialize emergency protocol
 */
void emergency_protocol_init(void)
{
    s_emergency_active = false;
    ESP_LOGI(TAG, "Emergency protocol initialized");
}

/**
 * @brief Clear emergency state
 */
void emergency_protocol_clear(void)
{
    if (s_emergency_active) {
        s_emergency_active = false;
        ESP_LOGI(TAG, "Emergency cleared");
    }
}

/**
 * @brief Get current emergency status
 */
bool emergency_protocol_is_active(void)
{
    return s_emergency_active;
}

/**
 * @brief Check if emergency condition exists
 */
bool emergency_protocol_check(const sensor_data_t *data)
{
    if (data == NULL) {
        return false;
    }

    bool emergency = false;

    /* Critical high temperature */
    if (data->temperature > EMERGENCY_TEMP_HIGH) {
        ESP_LOGE(TAG, "EMERGENCY: Temperature %.1f°C > %.1f°C!",
                 data->temperature, EMERGENCY_TEMP_HIGH);
        emergency = true;
    }

    /* Critical low temperature */
    if (data->temperature < EMERGENCY_TEMP_LOW) {
        ESP_LOGE(TAG, "EMERGENCY: Temperature %.1f°C < %.1f°C!",
                 data->temperature, EMERGENCY_TEMP_LOW);
        emergency = true;
    }

    /* Critical humidity */
    if (data->humidity > EMERGENCY_HUMIDITY_HIGH) {
        ESP_LOGE(TAG, "EMERGENCY: Humidity %.1f%% > %.1f%%!",
                 data->humidity, EMERGENCY_HUMIDITY_HIGH);
        emergency = true;
    }

    /* Critical CO */
    if (data->co_ppm > EMERGENCY_CO_CRITICAL) {
        ESP_LOGE(TAG, "EMERGENCY: CO %.1f ppm > %.1f ppm!",
                 data->co_ppm, EMERGENCY_CO_CRITICAL);
        emergency = true;
    }

    /* Critical ammonia */
    if (data->ammonia_ppm > EMERGENCY_AMMONIA_CRITICAL) {
        ESP_LOGE(TAG, "EMERGENCY: Ammonia %.1f ppm > %.1f ppm!",
                 data->ammonia_ppm, EMERGENCY_AMMONIA_CRITICAL);
        emergency = true;
    }

    /* Critical H2S */
    if (data->h2s_ppm > EMERGENCY_H2S_CRITICAL) {
        ESP_LOGE(TAG, "EMERGENCY: H2S %.1f ppm > %.1f ppm!",
                 data->h2s_ppm, EMERGENCY_H2S_CRITICAL);
        emergency = true;
    }

    if (emergency && !s_emergency_active) {
        s_emergency_active = true;
        ESP_LOGE(TAG, "*** EMERGENCY PROTOCOL ACTIVATED ***");
        /* Activate maximum ventilation and alarm */
        actuator_manager_set(ACTUATOR_TYPE_FAN1, 100);
        actuator_manager_set(ACTUATOR_TYPE_FAN2, 100);
        actuator_manager_set(ACTUATOR_TYPE_HEATER, 0);
        actuator_manager_set(ACTUATOR_TYPE_BUZZER, 1);
    } else if (!emergency && s_emergency_active) {
        /* Conditions returned to normal */
        emergency_protocol_clear();
        actuator_manager_set(ACTUATOR_TYPE_BUZZER, 0);
    }

    return emergency;
}