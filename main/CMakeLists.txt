# Main component CMakeLists.txt

idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "include"
    REQUIRES esp32 sensors actuators control communication storage utils nvs esp_http_client mqtt esp_idf_lib_hx711 esp_idf_lib_ds18x20
)
