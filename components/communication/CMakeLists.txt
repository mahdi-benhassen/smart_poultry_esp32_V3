# Communication Component CMakeLists.txt
# Smart Poultry V3 - Modular Architecture

set(COMM_SRCS "comm_manager.c")
set(COMM_REQUIRES esp_wifi esp_http_client mqtt nvs_flash)

if(CONFIG_ENABLE_COMM_WIFI)
    list(APPEND COMM_SRCS "wifi_manager.c")
endif()

if(CONFIG_ENABLE_COMM_MQTT)
    list(APPEND COMM_SRCS "mqtt_client.c")
endif()

if(CONFIG_ENABLE_COMM_OTA)
    list(APPEND COMM_SRCS "ota_update.c")
    list(APPEND COMM_REQUIRES esp_https_ota)
endif()

idf_component_register(
    SRCS ${COMM_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES ${COMM_REQUIRES}
)
