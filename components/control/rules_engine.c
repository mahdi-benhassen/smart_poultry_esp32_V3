/**
 * Rules Engine - Implementation
 */

#include <string.h>
#include <esp_log.h>
#include "rules_engine.h"

static const char *TAG = "RULES_ENGINE";

static rules_engine_thresholds_t s_thresholds = {
    .temp_min = 18.0f,
    .temp_max = 32.0f,
    .temp_optimal = 25.0f,
    .humidity_min = 40.0f,
    .humidity_max = 80.0f,
    .ammonia_max = 25.0f,
    .co_max = 10.0f
};

/**
 * @brief Initialize rules engine
 */
void rules_engine_init(void)
{
    ESP_LOGI(TAG, "Rules engine initialized");
}

/**
 * @brief Set thresholds
 */
void rules_engine_set_thresholds(const rules_engine_thresholds_t *thresholds)
{
    if (thresholds) {
        memcpy(&s_thresholds, thresholds, sizeof(rules_engine_thresholds_t));
    }
}

/**
 * @brief Evaluate rules
 */
void rules_engine_evaluate(const sensor_data_t *data, rules_engine_result_t *result)
{
    memset(result, 0, sizeof(rules_engine_result_t));
    
    /* Temperature rules */
    if (data->temperature > s_thresholds.temp_max) {
        result->temp_high = true;
    } else if (data->temperature < s_thresholds.temp_min) {
        result->temp_low = true;
    }
    
    /* Humidity rules */
    if (data->humidity > s_thresholds.humidity_max) {
        result->humidity_high = true;
    } else if (data->humidity < s_thresholds.humidity_min) {
        result->humidity_low = true;
    }
    
    /* Gas rules */
    if (data->ammonia_ppm > s_thresholds.ammonia_max) {
        result->ammonia_high = true;
    }
    
    if (data->co_ppm > s_thresholds.co_max) {
        result->co_high = true;
    }
    
    /* Emergency conditions */
    if (result->temp_high || result->co_high || result->ammonia_high) {
        if (data->temperature > s_thresholds.temp_max + 5.0f ||
            data->co_ppm > s_thresholds.co_max * 2.0f) {
            result->emergency = true;
        }
    }
}
