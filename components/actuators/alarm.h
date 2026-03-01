/**
 * Alarm (Buzzer/LED) Driver
 */

#ifndef ALARM_H
#define ALARM_H

#include <stdint.h>
#include <esp_err.h>

/**
 * @brief Initialize alarm
 */
esp_err_t alarm_init(void);

/**
 * @brief Set alarm on/off
 */
esp_err_t alarm_set(bool on);

/**
 * @brief Get alarm state
 */
esp_err_t alarm_get(bool *on);

#endif /* ALARM_H */
