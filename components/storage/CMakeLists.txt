# Storage Component CMakeLists.txt

idf_component_register(
    SRCS 
        "storage_manager.c"
        "nvs_storage.c"
    INCLUDE_DIRS 
        "include"
    REQUIRES nvs spiffs
)
