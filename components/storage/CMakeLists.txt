# Storage Component CMakeLists.txt
# Smart Poultry V3 - Modular Architecture

set(STORAGE_SRCS "storage_manager.c")

# Always require nvs_flash since storage_manager.c uses it conditionally
list(APPEND STORAGE_REQUIRES nvs_flash)

if(CONFIG_ENABLE_STORAGE_NVS)
    list(APPEND STORAGE_SRCS "nvs_storage.c")
endif()

if(CONFIG_ENABLE_STORAGE_SPIFFS)
    list(APPEND STORAGE_REQUIRES spiffs)
endif()

idf_component_register(
    SRCS ${STORAGE_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES ${STORAGE_REQUIRES}
)
