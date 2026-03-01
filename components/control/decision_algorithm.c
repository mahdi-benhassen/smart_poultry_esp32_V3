/**
 * Decision Algorithm - Implementation
 * Tunisian Mediterranean Climate Poultry Standards
 */

#include <string.h>
#include <esp_log.h>
#include "decision_algorithm.h"
#include "pid_controller.h"
#include "../../main/include/main.h"

static const char *TAG = "DECISION_ALGO";

/* Global flock state - managed externally */
extern system_state_t g_system_state;

/* Threshold for deactivating feeder */
#define FEEDER_TEMP_MAX 35.0f
#define FEEDER_TEMP_MIN 15.0f
#define FEEDER_HUMIDITY_MAX 85.0f
#define FEEDER_GAS_MAX 30.0f
#define FEEDING_INTERVAL_H 6

static uint32_t s_last_feeding_time = 0;

/**
 * @brief Get optimal temperature based on flock age and type
 */
static void get_optimal_temperature(float *min_temp, float *max_temp, float *opt_temp)
{
    uint16_t flock_age = g_system_state.flock_age_days;
    flock_type_t flock_type = g_system_state.flock_type;
    
    if (flock_type == FLOCK_TYPE_LAYER) {
        /* Adult Layers: 18-24°C */
        *min_temp = TEMP_LAYER_MIN;
        *max_temp = TEMP_LAYER_MAX;
        *opt_temp = TEMP_LAYER_OPT;
    } else {
        /* Broilers - Age based */
        if (flock_age <= 3) {
            /* Day 1-3 (Brooding): 32-34°C */
            *min_temp = TEMP_BROODING_DAY1_3_MIN;
            *max_temp = TEMP_BROODING_DAY1_3_MAX;
            *opt_temp = TEMP_BROODING_DAY1_3_OPT;
        } else if (flock_age <= 14) {
            /* Day 4-14: Decrease 0.5°C/day from 32°C */
            /* Day 4: ~30°C, Day 14: ~23°C */
            *min_temp = TEMP_BROODING_DAY4_MIN;
            *max_temp = TEMP_BROODING_DAY4_MAX;
            *opt_temp = TEMP_BROODING_DAY4_OPT;
        } else if (flock_age <= 21) {
            /* Week 3: 26-28°C */
            *min_temp = TEMP_WEEK3_MIN;
            *max_temp = TEMP_WEEK3_MAX;
            *opt_temp = TEMP_WEEK3_OPT;
        } else {
            /* Week 4+ (Broilers): 18-22°C */
            *min_temp = TEMP_BROILER_MIN;
            *max_temp = TEMP_BROILER_MAX;
            *opt_temp = TEMP_BROILER_OPT;
        }
    }
    
    ESP_LOGD(TAG, "Temperature targets for age %d days: min=%.1f, max=%.1f, opt=%.1f",
             flock_age, *min_temp, *max_temp, *opt_temp);
}

/**
 * @brief Get optimal humidity
 */
static float get_optimal_humidity(void)
{
    return HUMIDITY_OPTIMAL;
}

/**
 * @brief Get light requirements based on flock type and age
 */
static void get_light_requirements(float *min_lux, float *max_lux, uint8_t *on_hours, uint8_t *off_hours)
{
    flock_type_t flock_type = g_system_state.flock_type;
    uint16_t flock_age = g_system_state.flock_age_days;
    
    if (flock_type == FLOCK_TYPE_LAYER) {
        /* Layers: 10-30 lux, 14-16h continuous light */
        *min_lux = LIGHT_LAYER_MIN;
        *max_lux = LIGHT_LAYER_MAX;
        *on_hours = LIGHT_LAYER_ON_HOURS_MAX;  /* 16 hours for layers */
        *off_hours = 24 - *on_hours;
    } else {
        /* Broilers */
        if (flock_age <= 7) {
            /* Day 1-7: 20-30 lux */
            *min_lux = LIGHT_BROILER_DAY1_7_MIN;
            *max_lux = LIGHT_BROILER_DAY1_7_MAX;
        } else {
            /* After Day 7: 5-10 lux */
            *min_lux = LIGHT_BROILER_DAY7_MIN;
            *max_lux = LIGHT_BROILER_DAY7_MAX;
        }
        *on_hours = LIGHT_BROILER_ON_HOURS;   /* 18h ON */
        *off_hours = LIGHT_BROILER_OFF_HOURS; /* 6h OFF */
    }
    
    ESP_LOGD(TAG, "Light requirements: %.0f-%.0f lux, ON=%dh OFF=%dh",
             *min_lux, *max_lux, *on_hours, *off_hours);
}

/**
 * @brief Initialize decision algorithm
 */
void decision_algorithm_init(void)
{
    s_last_feeding_time = 0;
    ESP_LOGI(TAG, "Decision algorithm initialized with Tunisian standards");
}

/**
 * @brief Set flock age
 */
void decision_algorithm_set_flock_age(uint16_t age_days)
{
    g_system_state.flock_age_days = age_days;
    ESP_LOGI(TAG, "Flock age set to %d days", age_days);
}

/**
 * @brief Set flock type
 */
void decision_algorithm_set_flock_type(flock_type_t type)
{
    g_system_state.flock_type = type;
    ESP_LOGI(TAG, "Flock type set to %s", type == FLOCK_TYPE_BROILER ? "Broiler" : "Layer");
}

/**
 * @brief Evaluate and make decision
 */
void decision_algorithm_evaluate(const sensor_data_t *data, decision_output_t *output)
{
    memset(output, 0, sizeof(decision_output_t));
    
    float temp_min, temp_max, temp_opt;
    float humidity_opt;
    float light_min, light_max;
    uint8_t light_on_h, light_off_h;
    
    /* Get age-based temperature targets */
    get_optimal_temperature(&temp_min, &temp_max, &temp_opt);
    humidity_opt = get_optimal_humidity();
    get_light_requirements(&light_min, &light_max, &light_on_h, &light_off_h);
    
    /* Temperature control using PID with age-based targets */
    float temp_output = pid_controller_compute(temp_opt, data->temperature);
    
    if (temp_output > 0) {
        /* Temperature is too low - need heating */
        output->heater_level = (uint8_t)temp_output;
        output->fan1_speed = 20;  /* Low ventilation */
        output->fan2_speed = 20;
        ESP_LOGD(TAG, "Heating: level=%d, temp=%.1f, target=%.1f", 
                 output->heater_level, data->temperature, temp_opt);
    } else if (temp_output < -10) {
        /* Temperature is too high - need cooling */
        output->cooling_pad_on = true;
        output->fan1_speed = (uint8_t)(-temp_output);
        output->fan2_speed = (uint8_t)(-temp_output);
        ESP_LOGD(TAG, "Cooling: fans=%d, temp=%.1f, target=%.1f", 
                 output->fan1_speed, data->temperature, temp_opt);
    } else {
        /* Temperature is within acceptable range */
        output->fan1_speed = 30;
        output->fan2_speed = 30;
    }
    
    /* =========================================================================
     * HUMIDITY CONTROL (Tunisian Standards: 50-70% optimal, >75% critical)
     * ========================================================================= */
    if (data->humidity < humidity_opt - 10) {
        /* Too dry - turn on cooling pad for humidity */
        output->cooling_pad_on = true;
        ESP_LOGD(TAG, "Low humidity: %.1f%% - activating cooling pad", data->humidity);
    } else if (data->humidity > HUMIDITY_MAX + 5) {
        /* Too humid - increase ventilation (>75% triggers fans) */
        output->fan1_speed = 80;
        output->fan2_speed = 80;
        ESP_LOGD(TAG, "High humidity: %.1f%% - max ventilation", data->humidity);
    } else if (data->humidity > HUMIDITY_CRITICAL_HIGH) {
        /* Critical humidity - maximum ventilation */
        output->fan1_speed = 100;
        output->fan2_speed = 100;
        ESP_LOGW(TAG, "CRITICAL humidity: %.1f%%!", data->humidity);
    }
    
    /* =========================================================================
     * AIR QUALITY CONTROL (Tunisian Standards)
     * ========================================================================= */
    
    /* Ammonia (NH3): Target <10ppm, Warning >20ppm, Critical >25ppm */
    if (data->ammonia_ppm > AMMONIA_WARNING) {
        output->fan1_speed = 100;
        output->fan2_speed = 100;
        ESP_LOGW(TAG, "High ammonia: %.1f ppm - max ventilation", data->ammonia_ppm);
    } else if (data->ammonia_ppm > AMMONIA_TARGET) {
        output->fan1_speed = 80;
        output->fan2_speed = 80;
        ESP_LOGD(TAG, "Elevated ammonia: %.1f ppm", data->ammonia_ppm);
    }
    
    /* CO: Target 0ppm, Critical >10ppm */
    if (data->co_ppm > CO_WARNING) {
        output->fan1_speed = 100;
        output->fan2_speed = 100;
        ESP_LOGW(TAG, "HIGH CO: %.1f ppm - emergency ventilation!", data->co_ppm);
    } else if (data->co_ppm > CO_TARGET + 2) {
        output->fan1_speed = 80;
        output->fan2_speed = 80;
        ESP_LOGD(TAG, "Elevated CO: %.1f ppm", data->co_ppm);
    }
    
    /* CO2: Target <2500ppm, Critical >3000ppm */
    if (data->co2_ppm > CO2_CRITICAL) {
        output->fan1_speed = 100;
        output->fan2_speed = 100;
        ESP_LOGW(TAG, "CRITICAL CO2: %.1f ppm!", data->co2_ppm);
    } else if (data->co2_ppm > CO2_TARGET) {
        output->fan1_speed = 70;
        output->fan2_speed = 70;
        ESP_LOGD(TAG, "Elevated CO2: %.1f ppm", data->co2_ppm);
    }
    
    /* H2S: Critical >2ppm (NEW) */
    if (data->h2s_ppm > H2S_CRITICAL) {
        output->fan1_speed = 100;
        output->fan2_speed = 100;
        ESP_LOGW(TAG, "CRITICAL H2S: %.1f ppm!", data->h2s_ppm);
    } else if (data->h2s_ppm > H2S_WARNING) {
        output->fan1_speed = 90;
        output->fan2_speed = 90;
        ESP_LOGW(TAG, "High H2S: %.1f ppm", data->h2s_ppm);
    }
    
    /* =========================================================================
     * LIGHT CONTROL (Tunisian Standards)
     * ========================================================================= */
    /* Convert lux to percentage (0-100%) - assuming 100 lux = 100% */
    /* Broilers: Days 1-7: 20-30 lux, after Day 7: 5-10 lux */
    /* Layers: 10-30 lux */
    float light_percent = (light_min + light_max) / 2.0f;  /* Midpoint as target */
    output->light_level = (uint8_t)light_percent;
    ESP_LOGD(TAG, "Light level: %d%% (target %.0f-%.0f lux)", 
             output->light_level, light_min, light_max);
    
    /* =========================================================================
     * WATER MONITORING (Tunisian Standards)
     * ========================================================================= */
    /* Check water temperature */
    if (data->water_temperature > 0) {  /* Only if water temp sensor available */
        if (data->water_temperature > WATER_TEMP_MAX) {
            ESP_LOGW(TAG, "Water temperature HIGH: %.1f°C", data->water_temperature);
        } else if (data->water_temperature < WATER_TEMP_MIN) {
            ESP_LOGW(TAG, "Water temperature LOW: %.1f°C", data->water_temperature);
        }
    }
    
    /* Water pump - always on if water level is low */
    if (data->water_level < 30) {
        output->pump_on = true;
        ESP_LOGD(TAG, "Low water level: %d%% - pump on", data->water_level);
    }
    
    /* =========================================================================
     * FEEDER CONTROL
     * ========================================================================= */
    /* Deactivate feeder if temperature or humidity is extreme */
    if (data->temperature > FEEDER_TEMP_MAX || data->temperature < FEEDER_TEMP_MIN) {
        output->feeder_activate = false;
        ESP_LOGD(TAG, "Feeder deactivated: temperature out of range (%.1f°C)", data->temperature);
    } else if (data->humidity > FEEDER_HUMIDITY_MAX) {
        output->feeder_activate = false;
        ESP_LOGD(TAG, "Feeder deactivated: humidity too high (%.1f%%)", data->humidity);
    } else if (data->ammonia_ppm > FEEDER_GAS_MAX || data->co_ppm > FEEDER_GAS_MAX) {
        output->feeder_activate = false;
        ESP_LOGD(TAG, "Feeder deactivated: gas levels too high (NH3: %.1f, CO: %.1f)", 
                 data->ammonia_ppm, data->co_ppm);
    } else {
        /* Check if it's feeding time */
        uint32_t now = esp_log_timestamp() / 1000;  /* Convert to seconds */
        uint32_t hours_since_last = (now - s_last_feeding_time) / 3600;
        if (hours_since_last >= FEEDING_INTERVAL_H) {
            output->feeder_activate = true;
            s_last_feeding_time = now;
            ESP_LOGI(TAG, "Feeder activated - feeding time");
        }
    }
}

/**
 * @brief Check water consumption alert
 * @return true if consumption dropped >10%
 */
bool decision_algorithm_check_water_alert(float current_consumption)
{
    if (g_system_state.previous_water_consumption > 0) {
        float drop_percent = ((g_system_state.previous_water_consumption - current_consumption) 
                              / g_system_state.previous_water_consumption) * 100.0f;
        
        if (drop_percent > WATER_CONSUMPTION_DROP_ALERT) {
            ESP_LOGW(TAG, "Water consumption alert: dropped by %.1f%% (prev: %.1f, curr: %.1f)",
                     drop_percent, g_system_state.previous_water_consumption, current_consumption);
            return true;
        }
    }
    
    g_system_state.previous_water_consumption = current_consumption;
    return false;
}
