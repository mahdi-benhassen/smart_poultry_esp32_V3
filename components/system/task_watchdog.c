/**
 * Task Watchdog - Implementation
 * FreeRTOS Task Health Monitoring System
 */

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include "task_watchdog.h"

static const char *TAG = "WATCHDOG";

/* Watchdog state */
static watchdog_task_t watched_tasks[MAX_WATCHED_TASKS];
static size_t num_watched_tasks = 0;
static TimerHandle_t watchdog_timer = NULL;
static SemaphoreHandle_t watchdog_mutex = NULL;

/**
 * @brief Watchdog timer callback
 */
static void watchdog_timer_callback(TimerHandle_t xTimer)
{
    task_watchdog_check_all();
}

/**
 * @brief Initialize the task watchdog system
 */
esp_err_t task_watchdog_init(void)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG, "Initializing task watchdog system");

    /* Initialize task array */
    memset(watched_tasks, 0, sizeof(watched_tasks));
    num_watched_tasks = 0;

    /* Create mutex for thread safety */
    watchdog_mutex = xSemaphoreCreateMutex();
    if (watchdog_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create watchdog mutex");
        return ESP_ERR_NO_MEM;
    }

    /* Create watchdog timer */
    watchdog_timer = xTimerCreate(
        "WatchdogTimer",
        pdMS_TO_TICKS(WATCHDOG_CHECK_INTERVAL_MS),
        pdTRUE,
        NULL,
        watchdog_timer_callback
    );

    if (watchdog_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create watchdog timer");
        vSemaphoreDelete(watchdog_mutex);
        return ESP_ERR_NO_MEM;
    }

    /* Start the timer */
    if (xTimerStart(watchdog_timer, 100) != pdPASS) {
        ESP_LOGE(TAG, "Failed to start watchdog timer");
        vTimerDelete(watchdog_timer);
        vSemaphoreDelete(watchdog_mutex);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Task watchdog system initialized successfully");

    return ret;
}

/**
 * @brief Add a task to the watchdog monitor
 */
esp_err_t task_watchdog_add_task(TaskHandle_t task_handle, const char *task_name,
                                uint32_t timeout_ms, watchdog_severity_t severity)
{
    esp_err_t ret = ESP_OK;

    if (task_handle == NULL || task_name == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (num_watched_tasks >= MAX_WATCHED_TASKS) {
        ESP_LOGW(TAG, "Maximum number of watched tasks reached");
        return ESP_ERR_NO_MEM;
    }

    xSemaphoreTake(watchdog_mutex, portMAX_DELAY);

    /* Check if task is already being watched */
    for (size_t i = 0; i < num_watched_tasks; i++) {
        if (watched_tasks[i].handle == task_handle) {
            ESP_LOGW(TAG, "Task %s is already being watched", task_name);
            xSemaphoreGive(watchdog_mutex);
            return ESP_ERR_INVALID_STATE;
        }
    }

    /* Add the task */
    watched_tasks[num_watched_tasks].handle = task_handle;
    watched_tasks[num_watched_tasks].name = task_name;
    watched_tasks[num_watched_tasks].last_pet_time = xTaskGetTickCount();
    watched_tasks[num_watched_tasks].timeout_ms = timeout_ms;
    watched_tasks[num_watched_tasks].severity = severity;
    watched_tasks[num_watched_tasks].restart_count = 0;
    watched_tasks[num_watched_tasks].enabled = true;

    num_watched_tasks++;

    ESP_LOGI(TAG, "Added task '%s' to watchdog (timeout=%lums, severity=%d)",
             task_name, timeout_ms, severity);

    xSemaphoreGive(watchdog_mutex);

    return ret;
}

/**
 * @brief Pet the watchdog (call this from monitored tasks)
 */
esp_err_t task_watchdog_pet(TaskHandle_t task_handle)
{
    esp_err_t ret = ESP_ERR_NOT_FOUND;

    if (task_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(watchdog_mutex, portMAX_DELAY);

    for (size_t i = 0; i < num_watched_tasks; i++) {
        if (watched_tasks[i].handle == task_handle && watched_tasks[i].enabled) {
            watched_tasks[i].last_pet_time = xTaskGetTickCount();
            ret = ESP_OK;
            break;
        }
    }

    xSemaphoreGive(watchdog_mutex);

    return ret;
}

/**
 * @brief Check if a task has timed out and handle accordingly
 */
static esp_err_t handle_timeout(watchdog_task_t *task)
{
    esp_err_t ret = ESP_OK;
    uint32_t current_time = xTaskGetTickCount();
    uint32_t elapsed = pdTICKS_TO_MS(current_time - task->last_pet_time);

    ESP_LOGW(TAG, "Task '%s' timed out (%lu ms elapsed)", task->name, elapsed);

    switch (task->severity) {
        case WATCHDOG_SEVERITY_LOW:
            ESP_LOGW(TAG, "Task '%s' warning level timeout", task->name);
            break;

        case WATCHDOG_SEVERITY_MEDIUM:
            ESP_LOGW(TAG, "Restarting task '%s'", task->name);
            if (task->handle && eTaskGetState(task->handle) != eDeleted) {
                task->restart_count++;
                vTaskDelete(task->handle);
                /* Restart the task would need to be handled by main task manager */
            }
            break;

        case WATCHDOG_SEVERITY_HIGH:
            ESP_LOGE(TAG, "Critical task '%s' failed, restarting system", task->name);
            esp_restart();
            break;

        default:
            break;
    }

    return ret;
}

/**
 * @brief Check all monitored tasks
 */
esp_err_t task_watchdog_check_all(void)
{
    esp_err_t ret = ESP_OK;
    uint32_t current_time = xTaskGetTickCount();

    xSemaphoreTake(watchdog_mutex, portMAX_DELAY);

    for (size_t i = 0; i < num_watched_tasks; i++) {
        if (!watched_tasks[i].enabled) {
            continue;
        }

        uint32_t elapsed = pdTICKS_TO_MS(current_time - watched_tasks[i].last_pet_time);

        if (elapsed > watched_tasks[i].timeout_ms) {
            ESP_LOGW(TAG, "Task '%s' exceeded timeout (%lu > %lu)",
                     watched_tasks[i].name, elapsed, watched_tasks[i].timeout_ms);
            handle_timeout(&watched_tasks[i]);
        } else if (elapsed > watched_tasks[i].timeout_ms / 2) {
            ESP_LOGW(TAG, "Task '%s' approaching timeout (%lu/%lu ms)",
                     watched_tasks[i].name, elapsed, watched_tasks[i].timeout_ms);
        }
    }

    xSemaphoreGive(watchdog_mutex);

    return ret;
}

/**
 * @brief Remove a task from monitoring
 */
esp_err_t task_watchdog_remove_task(TaskHandle_t task_handle)
{
    esp_err_t ret = ESP_ERR_NOT_FOUND;

    if (task_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(watchdog_mutex, portMAX_DELAY);

    for (size_t i = 0; i < num_watched_tasks; i++) {
        if (watched_tasks[i].handle == task_handle) {
            ESP_LOGI(TAG, "Removing task '%s' from watchdog", watched_tasks[i].name);

            /* Shift remaining tasks */
            for (size_t j = i; j < num_watched_tasks - 1; j++) {
                watched_tasks[j] = watched_tasks[j + 1];
            }

            num_watched_tasks--;
            ret = ESP_OK;
            break;
        }
    }

    xSemaphoreGive(watchdog_mutex);

    return ret;
}

/**
 * @brief Enable/disable watchdog for a task
 */
esp_err_t task_watchdog_enable_task(TaskHandle_t task_handle, bool enable)
{
    esp_err_t ret = ESP_ERR_NOT_FOUND;

    if (task_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(watchdog_mutex, portMAX_DELAY);

    for (size_t i = 0; i < num_watched_tasks; i++) {
        if (watched_tasks[i].handle == task_handle) {
            watched_tasks[i].enabled = enable;
            if (enable) {
                /* Reset pet time when re-enabling */
                watched_tasks[i].last_pet_time = xTaskGetTickCount();
            }
            ESP_LOGI(TAG, "Task '%s' %s", watched_tasks[i].name,
                     enable ? "enabled" : "disabled");
            ret = ESP_OK;
            break;
        }
    }

    xSemaphoreGive(watchdog_mutex);

    return ret;
}

/**
 * @brief Get task status information
 */
esp_err_t task_watchdog_get_status(watchdog_task_t *status_array, size_t *count)
{
    if (status_array == NULL || count == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(watchdog_mutex, portMAX_DELAY);

    size_t copy_count = (*count > num_watched_tasks) ? num_watched_tasks : *count;
    memcpy(status_array, watched_tasks, copy_count * sizeof(watchdog_task_t));
    *count = copy_count;

    xSemaphoreGive(watchdog_mutex);

    return ESP_OK;
}