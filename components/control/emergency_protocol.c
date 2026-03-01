/**
 * Emergency Protocol - Implementation
 * Tunisian Mediterranean Climate Poultry Standards
 */

#include "emergency_protocol.h"
#include "actuator_manager.h"
#include "main/include/main.h"

static const char *TAG = "EMERGENCY";

static bool s_emergency_active = false;
static uint32_t s_emergency_start_time = 0;
#define EMERGENCY_AUTO_RESET_MS 30000  /* Auto-reset after 30 seconds */

/**
 * @brief Initialize emergency protocol
 */
void emergency_protocol_init(void)
{
    s_emergency_active = false;
    s_emergency_start_time = 0;
    ESP_LOGI(TAG, "Emergency protocol initialized with Tunisian standards");
}

/**
 * @brief Clear emergency state and re-enable actuators
 */
void emergency_protocol_clear(void)
{
    if (s_emergency_active) {
        s_emergency_active = false;
        s_emergency_start_time = 0;
        ESP_LOGI(TAG, "Emergency state cleared - actuators re-enabled");
    }
}

/**
 * @brief Get emergency status
 */
bool emergency_protocol_is_active(void)
{
    return s_emergency_active;
}

/**
 * @brief Check if emergency condition exists
 * Implements Tunisian Mediterranean Climate Poultry Standards
 */
bool emergency_protocol_check(const sensor_data_t *data)
{
    if (!data) return false;
    
    /* Check if we can auto-reset from emergency */
    if (s_emergency_active && s_emergency_start_time > 0) {
        uint32_t now = esp_log_timestamp();
        if (now - s_emergency_start_time > EMERGENCY_AUTO_RESET_MS) {
            /* Check if conditions have stabilized using Tunisian thresholds */
            if (data->temperature > EMERGENCY_TEMP_LOW + 5.0f && 
                data->temperature < EMERGENCY_TEMP_HIGH - 2.0f &&
                data->co_ppm < EMERGENCY_CO_CRITICAL - 10.0f && 
                data->ammonia_ppm < EMERGENCY_AMMONIA_CRITICAL - 20.0f &&
                data->h2s_ppm < EMERGENCY_H2S_CRITICAL - 1.0f) {
                emergency_protocol_clear();
                return false;
            }
        }
    }
    
    /* =========================================================================
     * CRITICAL EMERGENCY CHECKS (Tunisian Standards)
     * ========================================================================= */
    
    /* Temperature emergencies */
    if (data->temperature > EMERGENCY_TEMP_HIGH) {
        if (!s_emergency_active) {
            s_emergency_start_time = esp_log_timestamp();
        }
        s_emergency_active = true;
        ESP_LOGE(TAG, "EMERGENCY: Temperature too HIGH! (%.1f°C > %.1f°C)", 
                 data->temperature, EMERGENCY_TEMP_HIGH);
    } else if (data->temperature < EMERGENCY_TEMP_LOW) {
        if (!s_emergency_active) {
            s_emergency_start_time = esp_log_timestamp();
        }
        s_emergency_active = true;
        ESP_LOGE(TAG, "EMERGENCY: Temperature too LOW! (%.1f°C < %.1f°C)", 
                 data->temperature, EMERGENCY_TEMP_LOW);
    }
    
    /* Humidity emergency (>85%) */
    else if (data->humidity > EMERGENCY_HUMIDITY_HIGH) {
        if (!s_emergency_active) {
            s_emergency_start_time = esp_log_timestamp();
        }
        s_emergency_active = true;
        ESP_LOGE(TAG, "EMERGENCY: Humidity too HIGH! (%.1f%% > %.1f%%)", 
                 data->humidity, EMERGENCY_HUMIDITY_HIGH);
    }
    
    /* CO emergency (>30ppm) - Critical according to Tunisian standards */
    else if (data->co_ppm > EMERGENCY_CO_CRITICAL) {
        if (!s_emergency_active) {
            s_emergency_start_time = esp_log_timestamp();
        }
        s_emergency_active = true;
        ESP_LOGE(TAG, "EMERGENCY: Carbon Monoxide CRITICAL! (%.1f ppm > %.1f ppm)", 
                 data->co_ppm, EMERGENCY_CO_CRITICAL);
    }
    
    /* Ammonia emergency (>50ppm) - Critical according to Tunisian standards */
    else if (data->ammonia_ppm > EMERGENCY_AMMONIA_CRITICAL) {
        if (!s_emergency_active) {
            s_emergency_start_time = esp_log_timestamp();
        }
        s_emergency_active = true;
        ESP_LOGE(TAG, "EMERGENCY: Ammonia CRITICAL! (%.1f ppm > %.1f ppm)", 
                 data->ammonia_ppm, EMERGENCY_AMMONIA_CRITICAL);
    }
    
    /* H2S emergency (>5ppm) - NEW for Tunisian standards */
    else if (data->h2s_ppm > EMERGENCY_H2S_CRITICAL) {
        if (!s_emergency_active) {
            s_emergency_start_time = esp_log_timestamp();
        }
        s_emergency_active = true;
        ESP_LOGE(TAG, "EMERGENCY: Hydrogen Sulfide CRITICAL! (%.1f ppm > %.1f ppm)", 
                 data->h2s_ppm, EMERGENCY_H2S_CRITICAL);
    }
    
    /* CO2 critical (>3500ppm) */
    else if (data->co2_ppm > 3500.0f) {
        if (!s_emergency_active) {
            s_emergency_start_time = esp_log_timestamp();
        }
        s_emergency_active = true;
        ESP_LOGE(TAG, "EMERGENCY: CO2 CRITICAL! (%.1f ppm)", data->co2_ppm);
    }
    
    if (s_emergency_active) {
        /* Activate emergency response - stop all actuators */
        actuator_manager_emergency_stop();
    }
    
    return s_emergency_active;
}
