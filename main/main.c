/**
 * Smart Poultry System - Main Application
 * ESP32-based environmental monitoring and control system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>

#include "main.h"
#include "sensor_manager.h"
#include "actuator_manager.h"
#include "control_manager.h"
#include "comm_manager.h"
#include "storage_manager.h"

static const char *TAG = "MAIN";

/* Task handles */
static TaskHandle_t sensor_task_handle = NULL;
static TaskHandle_t control_task_handle = NULL;
static TaskHandle_t comm_task_handle = NULL;
static TaskHandle_t storage_task_handle = NULL;

/* Task priorities */
#define SENSOR_TASK_PRIORITY    5
#define CONTROL_TASK_PRIORITY   6
#define COMM_TASK_PRIORITY      4
#define STORAGE_TASK_PRIORITY   3
#define SENSOR_TASK_STACK       4096
#define CONTROL_TASK_STACK      4096
#define COMM_TASK_STACK         4096
#define STORAGE_TASK_STACK      3072

/* Control loop interval in ms */
#define CONTROL_LOOP_INTERVAL   5000    /* 5 seconds */
#define SENSOR_READ_INTERVAL    3000    /* 3 seconds */
#define MQTT_PUBLISH_INTERVAL   30000   /* 30 seconds */
#define STORAGE_LOG_INTERVAL    60000   /* 60 seconds */

/* Global system state */
system_state_t g_system_state = {
    .initialized = false,
    .wifi_connected = false,
    .mqtt_connected = false,
    .emergency_mode = false,
    .last_sensor_read = 0,
    .last_control_update = 0,
    .last_mqtt_publish = 0,
    .last_storage_log = 0
};

/* Forward declarations */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data);
static void sensor_task(void *pvParameters);
static void control_task(void *pvParameters);
static void comm_task(void *pvParameters);
static void storage_task(void *pvParameters);

/**
 * @brief Initialize NVS storage
 */
static esp_err_t init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

/**
 * @brief Initialize WiFi
 */
static esp_err_t init_wifi(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                                               &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, 
                                                 &wifi_event_handler, NULL));
    
    /* Get WiFi credentials from NVS */
    storage_config_t wifi_config;
    if (storage_get_wifi_config(&wifi_config) == ESP_OK) {
        wifi_config_t sta_config = {
            .sta = {
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            },
        };
        memcpy(sta_config.sta.ssid, wifi_config.wifi_ssid, sizeof(wifi_config.wifi_ssid));
        memcpy(sta_config.sta.password, wifi_config.wifi_password, sizeof(wifi_config.wifi_password));
        
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
        ESP_ERROR_CHECK(esp_wifi_start());
        
        ESP_LOGI(TAG, "WiFi initialization complete, connecting to SSID: %s", 
                 wifi_config.wifi_ssid);
    } else {
        /* Fallback to default config */
        ESP_LOGW(TAG, "WiFi config not found, using default");
    }
    
    return ESP_OK;
}

/**
 * @brief WiFi event handler
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            g_system_state.wifi_connected = false;
            ESP_LOGI(TAG, "WiFi disconnected, reconnecting...");
            esp_wifi_connect();
            break;
        case IP_EVENT_STA_GOT_IP:
            g_system_state.wifi_connected = true;
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
            /* Initialize MQTT after getting IP */
            comm_manager_init_mqtt();
            break;
        default:
            break;
    }
}

/**
 * @brief Sensor reading task
 */
static void sensor_task(void *pvParameters)
{
    sensor_data_t sensor_data;
    uint32_t tick_count = 0;
    
    ESP_LOGI(TAG, "Sensor task started");
    
    while (1) {
        /* Read all sensors */
        if (sensor_manager_read_all(&sensor_data) == ESP_OK) {
            /* Store sensor data for control and communication */
            control_manager_update_sensors(&sensor_data);
            
            ESP_LOGD(TAG, "Sensor readings: Temp=%.1f, Humidity=%.1f, Ammonia=%.1f",
                     sensor_data.temperature, sensor_data.humidity, sensor_data.ammonia_ppm);
        } else {
            ESP_LOGW(TAG, "Failed to read sensors");
        }
        
        vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_INTERVAL));
        tick_count++;
    }
}

/**
 * @brief Control logic task
 */
static void control_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Control task started");
    
    while (1) {
        /* Run control loop */
        control_manager_update();
        
        vTaskDelay(pdMS_TO_TICKS(CONTROL_LOOP_INTERVAL));
    }
}

/**
 * @brief Communication task
 */
static void comm_task(void *pvParameters)
{
    uint32_t tick_count = 0;
    
    ESP_LOGI(TAG, "Communication task started");
    
    while (1) {
        /* Check MQTT connection and publish */
        if (g_system_state.wifi_connected && g_system_state.mqtt_connected) {
            /* Publish sensor data */
            if (tick_count % (MQTT_PUBLISH_INTERVAL / CONTROL_LOOP_INTERVAL) == 0) {
                sensor_data_t data;
                if (control_manager_get_sensor_data(&data) == ESP_OK) {
                    comm_manager_publish_sensors(&data);
                }
                
                /* Publish actuator state */
                actuator_state_t *act_state = actuator_manager_get_state();
                if (act_state != NULL) {
                    comm_manager_publish_actuators(act_state);
                }
            }
        }
        
        /* Check for OTA updates */
        comm_manager_check_ota();
        
        vTaskDelay(pdMS_TO_TICKS(CONTROL_LOOP_INTERVAL));
        tick_count++;
    }
}

/**
 * @brief Storage logging task
 */
static void storage_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Storage task started");
    
    while (1) {
        /* Log sensor data to storage */
        sensor_data_t data;
        if (control_manager_get_sensor_data(&data) == ESP_OK) {
            storage_log_sensor_data(&data);
        }
        
        vTaskDelay(pdMS_TO_TICKS(STORAGE_LOG_INTERVAL));
    }
}

/**
 * @brief Initialize all system components
 */
static esp_err_t init_system(void)
{
    esp_err_t ret;
    
    /* Initialize NVS */
    ret = init_nvs();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return ret;
    }
    
    /* Initialize storage */
    ret = storage_manager_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize storage: %s", esp_err_to_name(ret));
        return ret;
    }
    
    /* Initialize sensors */
    ret = sensor_manager_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize sensors: %s", esp_err_to_name(ret));
        return ret;
    }
    
    /* Initialize actuators */
    ret = actuator_manager_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize actuators: %s", esp_err_to_name(ret));
        return ret;
    }
    
    /* Initialize control system */
    ret = control_manager_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize control system: %s", esp_err_to_name(ret));
        return ret;
    }
    
    /* Initialize communication */
    ret = comm_manager_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize communication: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

/**
 * @brief Application entry point
 */
void app_main(void)
{
    esp_err_t ret;
    
    /* Print system info */
    ESP_LOGI(TAG, "Smart Poultry System v%s", SMART_POULTRY_VERSION);
    ESP_LOGI(TAG, "ESP32 Chip: %s", esp_get_idf_version());
    
    /* Initialize event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    /* Initialize all system components */
    ret = init_system();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "System initialization failed: %s", esp_err_to_name(ret));
        /* Continue anyway - some features may work */
    }
    
    /* Initialize WiFi */
    init_wifi();
    
    /* Create system tasks */
    xTaskCreate(sensor_task, "sensor_task", SENSOR_TASK_STACK, 
                NULL, SENSOR_TASK_PRIORITY, &sensor_task_handle);
    
    xTaskCreate(control_task, "control_task", CONTROL_TASK_STACK, 
                NULL, CONTROL_TASK_PRIORITY, &control_task_handle);
    
    xTaskCreate(comm_task, "comm_task", COMM_TASK_STACK, 
                NULL, COMM_TASK_PRIORITY, &comm_task_handle);
    
    xTaskCreate(storage_task, "storage_task", STORAGE_TASK_STACK, 
                NULL, STORAGE_TASK_PRIORITY, &storage_task_handle);
    
    /* Mark system as initialized */
    g_system_state.initialized = true;
    
    ESP_LOGI(TAG, "System initialization complete");
}
