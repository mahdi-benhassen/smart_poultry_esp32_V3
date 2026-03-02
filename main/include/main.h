/**
 * Smart Poultry System - Main Header
 * Tunisian Mediterranean Climate Poultry Standards
 */

#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>

#include "sensor_manager.h"

/* Version */
#define SMART_POULTRY_VERSION "1.1.0"

/* Flock Types */
typedef enum {
    FLOCK_TYPE_BROILER,    /* Broiler chickens */
    FLOCK_TYPE_LAYER       /* Layer hens */
} flock_type_t;

/* System State */
typedef struct {
    bool initialized;
    bool wifi_connected;
    bool mqtt_connected;
    bool emergency_mode;
    uint32_t last_sensor_read;
    uint32_t last_control_update;
    uint32_t last_mqtt_publish;
    uint32_t last_storage_log;
    /* Tunisian Standards: Flock Management */
    uint16_t flock_age_days;       /* Current flock age in days */
    flock_type_t flock_type;       /* Broiler or Layer */
    float previous_water_consumption;  /* For consumption monitoring */
} system_state_t;

/* Global system state */
extern system_state_t g_system_state;

/* Actuator State */
typedef struct {
    uint8_t fan1_speed;
    uint8_t fan2_speed;
    bool cooling_pad_on;
    uint8_t heater_level;
    uint8_t light_level;
    bool feeder_active;
    bool pump_on;
    bool alarm_active;
} actuator_state_t;

/* System Configuration */
typedef struct {
    /* Temperature thresholds */
    float temp_min;
    float temp_max;
    float temp_optimal;
    
    /* Humidity thresholds */
    float humidity_min;
    float humidity_max;
    
    /* Gas thresholds */
    float ammonia_max;
    float co_max;
    float co2_max;
    float h2s_max;
    float lpg_max;
    
    /* PID parameters */
    float pid_temp_kp;
    float pid_temp_ki;
    float pid_temp_kd;
    float pid_hum_kp;
    float pid_hum_ki;
    float pid_hum_kd;
    
    /* MQTT config */
    char mqtt_broker[128];
    char mqtt_topic[64];
    uint16_t mqtt_port;
    char mqtt_username[32];
    char mqtt_password[64];
    
    /* WiFi config */
    char wifi_ssid[32];
    char wifi_password[64];
    
    /* Device ID */
    char device_id[32];
    
    /* Light schedule */
    uint8_t light_on_hours;
    uint8_t light_off_hours;
} system_config_t;

/* ============================================================================
 * TUNISIAN MEDITERRANEAN CLIMATE POULTRY STANDARDS
 * ============================================================================ */

/* --- Temperature Control (Age-Based) --- */
/* Brooding (Day 1-3) */
#define TEMP_BROODING_DAY1_3_MIN    32.0f
#define TEMP_BROODING_DAY1_3_MAX    34.0f
#define TEMP_BROODING_DAY1_3_OPT    33.0f

/* Day 4-14: Decrease 0.5°C/day (from 32°C) */
#define TEMP_BROODING_DAY4_MIN      29.0f
#define TEMP_BROODING_DAY4_MAX      31.0f
#define TEMP_BROODING_DAY4_OPT      30.0f

/* Week 3 */
#define TEMP_WEEK3_MIN              26.0f
#define TEMP_WEEK3_MAX             28.0f
#define TEMP_WEEK3_OPT             27.0f

/* Week 4+ (Broilers) */
#define TEMP_BROILER_MIN           18.0f
#define TEMP_BROILER_MAX           22.0f
#define TEMP_BROILER_OPT           20.0f

/* Adult Layers */
#define TEMP_LAYER_MIN             18.0f
#define TEMP_LAYER_MAX             24.0f
#define TEMP_LAYER_OPT             21.0f

/* --- Humidity Control --- */
#define HUMIDITY_MIN                50.0f    /* Optimal minimum */
#define HUMIDITY_MAX                70.0f    /* Optimal maximum */
#define HUMIDITY_OPTIMAL            60.0f
#define HUMIDITY_CRITICAL_HIGH      75.0f    /* Triggers exhaust fans */

/* --- Air Quality Thresholds --- */
/* Ammonia (NH3) */
#define AMMONIA_TARGET              10.0f    /* Target <10ppm */
#define AMMONIA_WARNING             20.0f    /* Warning >20ppm */
#define AMMONIA_CRITICAL            25.0f    /* Critical >25ppm */

/* Carbon Monoxide (CO) */
#define CO_TARGET                   0.0f     /* Target 0ppm */
#define CO_WARNING                  5.0f     /* Warning >5ppm */
#define CO_CRITICAL                 10.0f    /* Critical >10ppm */

/* Carbon Dioxide (CO2) */
#define CO2_TARGET                  2500.0f /* Target <2500ppm */
#define CO2_WARNING                 2800.0f /* Warning >2800ppm */
#define CO2_CRITICAL                3000.0f /* Critical >3000ppm */

/* Hydrogen Sulfide (H2S) - NEW */
#define H2S_TARGET                  0.0f     /* Target 0ppm */
#define H2S_WARNING                 1.0f     /* Warning >1ppm */
#define H2S_CRITICAL                2.0f     /* Critical >2ppm */

/* --- Light Control --- */
/* Broilers */
#define LIGHT_BROILER_DAY1_7_MIN    20.0f    /* 20-30 lux Days 1-7 */
#define LIGHT_BROILER_DAY1_7_MAX    30.0f
#define LIGHT_BROILER_DAY7_MIN      5.0f     /* 5-10 lux after Day 7 */
#define LIGHT_BROILER_DAY7_MAX      10.0f
#define LIGHT_BROILER_ON_HOURS      18       /* 18h ON */
#define LIGHT_BROILER_OFF_HOURS     6        /* 6h OFF */

/* Layers */
#define LIGHT_LAYER_MIN             10.0f    /* 10-30 lux */
#define LIGHT_LAYER_MAX             30.0f
#define LIGHT_LAYER_ON_HOURS_MIN    14       /* 14-16h continuous */
#define LIGHT_LAYER_ON_HOURS_MAX    16

/* --- Water Monitoring --- */
#define WATER_TEMP_MIN              10.0f    /* Min water temperature */
#define WATER_TEMP_MAX              25.0f    /* Max water temperature */
#define WATER_TEMP_OPTIMAL          20.0f    /* Optimal water temperature */
#define WATER_CONSUMPTION_DROP_ALERT 10.0f   /* Alert if drop >10% */

/* --- Emergency Thresholds --- */
#define EMERGENCY_TEMP_HIGH         38.0f    /* Critical high temp */
#define EMERGENCY_TEMP_LOW          5.0f     /* Critical low temp */
#define EMERGENCY_HUMIDITY_HIGH     85.0f    /* Critical humidity */
#define EMERGENCY_CO_CRITICAL        30.0f    /* Critical CO */
#define EMERGENCY_AMMONIA_CRITICAL  50.0f    /* Critical ammonia */
#define EMERGENCY_H2S_CRITICAL       5.0f     /* Critical H2S */

/* Default configuration */
#define DEFAULT_TEMP_MIN         18.0f
#define DEFAULT_TEMP_MAX         32.0f
#define DEFAULT_TEMP_OPTIMAL     25.0f
#define DEFAULT_HUM_MIN          40.0f
#define DEFAULT_HUM_MAX         80.0f
#define DEFAULT_AMMONIA_MAX      25.0f
#define DEFAULT_CO_MAX           10.0f
#define DEFAULT_CO2_MAX          3000.0f
#define DEFAULT_H2S_MAX          2.0f
#define DEFAULT_LPG_MAX          5.0f

#define DEFAULT_PID_KP           2.0f
#define DEFAULT_PID_KI           0.5f
#define DEFAULT_PID_KD           0.1f

#define DEFAULT_MQTT_BROKER      "mqtt://localhost"
#define DEFAULT_MQTT_PORT        1883

/* PWM Configuration */
#define PWM_FREQ                 1000    /* 1 kHz */

#define GPIO_FAN1_PWM            12
#define GPIO_FAN1_EN             13
#define GPIO_FAN2_PWM            14
#define GPIO_FAN2_EN             15
#define GPIO_COOLING_PAD         16
#define GPIO_HEATER_PWM          17
#define GPIO_HEATER_EN           18
#define GPIO_LED_LIGHT           19
#define GPIO_FEEDER              21
#define GPIO_WATER_PUMP          22
#define GPIO_BUZZER              23
#define GPIO_LED_INDICATOR       25

/* PWM Configuration */
#define PWM_FREQ                 1000    /* 1 kHz */
#define PWM_RESOLUTION           10      /* 10-bit */

/* Number of sensors/actuators */
#define MAX_DS18B20_SENSORS      4
#define MAX_SENSOR_READINGS      10

/* Timeouts */
#define WIFI_CONNECT_TIMEOUT     30000   /* ms */
#define MQTT_CONNECT_TIMEOUT     10000   /* ms */
#define SENSOR_READ_TIMEOUT      5000    /* ms */

/* Error codes */
#define ERR_SENSOR_INVALID       0x01
#define ERR_SENSOR_TIMEOUT       0x02
#define ERR_ACTUATOR_FAILURE     0x03
#define ERR_WIFI_DISCONNECTED    0x04
#define ERR_MQTT_DISCONNECTED    0x05
#define ERR_STORAGE_FULL         0x06
#define ERR_WATER_TEMP_HIGH      0x07
#define ERR_WATER_CONSUMPTION    0x08
#define ERR_EMERGENCY_TRIGGERED  0x09

#endif /* MAIN_H */
