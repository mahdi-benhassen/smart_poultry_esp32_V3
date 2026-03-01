# Main component CMakeLists.txt

idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "include"
    REQUIRES sensors actuators control communication storage nvs esp_http_client mqtt
)
