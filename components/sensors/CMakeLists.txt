# Sensors Component CMakeLists.txt
# Smart Poultry V3 - Extended Sensor Support

# Build source list based on enabled sensors
set(SENSOR_SRCS "sensor_manager.c")

# Temperature/Humidity Sensors
if(CONFIG_ENABLE_SENSOR_DHT22)
    list(APPEND SENSOR_SRCS "dht22.c")
endif()

if(CONFIG_ENABLE_SENSOR_DS18B20)
    list(APPEND SENSOR_SRCS "ds18b20.c")
endif()

if(CONFIG_ENABLE_SENSOR_BMP280)
    list(APPEND SENSOR_SRCS "bmp280.c")
endif()

# Air Quality Sensors
if(CONFIG_ENABLE_SENSOR_MQ135)
    list(APPEND SENSOR_SRCS "mq135.c")
endif()

if(CONFIG_ENABLE_SENSOR_MQ7)
    list(APPEND SENSOR_SRCS "mq7.c")
endif()

if(CONFIG_ENABLE_SENSOR_MQ2)
    list(APPEND SENSOR_SRCS "mq2.c")
endif()

if(CONFIG_ENABLE_SENSOR_SGP30)
    list(APPEND SENSOR_SRCS "sgp30.c")
endif()

if(CONFIG_ENABLE_SENSOR_PMS5003)
    list(APPEND SENSOR_SRCS "pms5003.c")
endif()

# Light Sensor
if(CONFIG_ENABLE_SENSOR_LDR)
    list(APPEND SENSOR_SRCS "ldr.c")
endif()

# Water Level Sensor
if(CONFIG_ENABLE_SENSOR_WATER_LEVEL)
    list(APPEND SENSOR_SRCS "water_level.c")
endif()

# Sound Sensor
if(CONFIG_ENABLE_SENSOR_SOUND)
    list(APPEND SENSOR_SRCS "sound_sensor.c")
endif()

# Weight/Current Sensors
if(CONFIG_ENABLE_SENSOR_HX711)
    list(APPEND SENSOR_SRCS "hx711.c")
endif()

if(CONFIG_ENABLE_SENSOR_ACS712)
    list(APPEND SENSOR_SRCS "acs712.c")
endif()

idf_component_register(
    SRCS ${SENSOR_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES driver esp_adc nvs_flash esp_timer
)
