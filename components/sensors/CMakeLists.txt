# Sensors Component CMakeLists.txt

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
    INCLUDE_DIRS 
        "include"
    REQUIRES esp32 driver esp_adc_cal nvs
)
