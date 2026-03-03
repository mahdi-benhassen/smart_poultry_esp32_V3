# Sensors Component CMakeLists.txt
# Smart Poultry V3 - Extended Sensor Support

idf_component_register(
    SRCS 
        "sensor_manager.c"
        "dht22.c"
        "ds18b20.c"
        "mq135.c"
        "mq7.c"
        "mq2.c"
        "ldr.c"
        "water_level.c"
        "sound_sensor.c"
        # New sensors
        "bmp280.c"
        "sgp30.c"
        "pms5003.c"
        "hx711.c"
        "acs712.c"
    INCLUDE_DIRS 
        "include"
    REQUIRES driver esp_adc nvs_flash esp_timer esp_idf_lib_helpers
)
