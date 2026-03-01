/**
 * Rules Engine - Header File
 */

#ifndef RULES_ENGINE_H
#define RULES_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

/* Forward declaration */
typedef struct sensor_data sensor_data_t;

/* Rules result */
typedef struct {
    bool temp_high;
    bool temp_low;
    bool humidity_high;
    bool humidity_low;
    bool ammonia_high;
    bool co_high;
    bool emergency;
} rules_engine_result_t;

/* Thresholds */
typedef struct {
    float temp_min;
    float temp_max;
    float temp_optimal;
    float humidity_min;
    float humidity_max;
    float ammonia_max;
    float co_max;
} rules_engine_thresholds_t;

/**
 * @brief Initialize rules engine
 */
void rules_engine_init(void);

/**
 * @brief Set thresholds
 */
void rules_engine_set_thresholds(const rules_engine_thresholds_t *thresholds);

/**
 * @brief Evaluate rules
 */
void rules_engine_evaluate(const sensor_data_t *data, rules_engine_result_t *result);

#endif /* RULES_ENGINE_H */
