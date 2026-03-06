/**
 * Task Watchdog - Header File
 * FreeRTOS Task Health Monitoring System
 */

#ifndef TASK_WATCHDOG_H
#define TASK_WATCHDOG_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <esp_err.h>

/* Task watchdog configuration */
#define WATCHDOG_TIMEOUT_MS        60000    /* 60 seconds timeout */
#define WATCHDOG_CHECK_INTERVAL_MS 10000    /* Check every 10 seconds */
#define MAX_WATCHED_TASKS          8

/* Watchdog severity levels */
typedef enum {
    WATCHDOG_SEVERITY_LOW,      /* Warning only */
    WATCHDOG_SEVERITY_MEDIUM,   /* Restart task */
    WATCHDOG_SEVERITY_HIGH,     /* Restart system */
} watchdog_severity_t;

/* Task information for tracking */
typedef struct {
    TaskHandle_t handle;
    const char *name;
    uint32_t last_pet_time;
    uint32_t timeout_ms;
    watchdog_severity_t severity;
    uint32_t restart_count;
    bool enabled;
} watchdog_task_t;

/**
 * @brief Initialize the task watchdog system
 */
esp_err_t task_watchdog_init(void);

/**
 * @brief Add a task to the watchdog monitor
 */
esp_err_t task_watchdog_add_task(TaskHandle_t task_handle, const char *task_name,
                                uint32_t timeout_ms, watchdog_severity_t severity);

/**
 * @brief Pet the watchdog (call this from monitored tasks)
 */
esp_err_t task_watchdog_pet(TaskHandle_t task_handle);

/**
 * @brief Remove a task from monitoring
 */
esp_err_t task_watchdog_remove_task(TaskHandle_t task_handle);

/**
 * @brief Check all monitored tasks
 */
esp_err_t task_watchdog_check_all(void);

/**
 * @brief Enable/disable watchdog for a task
 */
esp_err_t task_watchdog_enable_task(TaskHandle_t task_handle, bool enable);

/**
 * @brief Get task status information
 */
esp_err_t task_watchdog_get_status(watchdog_task_t *status_array, size_t *count);

/* Convenience macros for tasks */
#define TASK_WATCHDOG_PET() task_watchdog_pet(xTaskGetCurrentTaskHandle())

#endif /* TASK_WATCHDOG_H */