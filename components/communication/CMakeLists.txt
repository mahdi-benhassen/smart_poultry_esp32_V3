# Communication Component CMakeLists.txt

idf_component_register(
    SRCS 
        "wifi_manager.c"
        "mqtt_client.c"
        "ota_update.c"
        "comm_manager.c"
    INCLUDE_DIRS 
        "include"
    REQUIRES esp_wifi esp_http_client mqtt nvs_flash
)
