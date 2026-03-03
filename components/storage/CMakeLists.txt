# Storage Component CMakeLists.txt
# Smart Poultry V3 - Modular Architecture

set(STORAGE_SRCS "storage_manager.c")
set(STORAGE_REQUIRES)

if(CONFIG_ENABLE_STORAGE_NVS)
    list(APPEND STORAGE_SRCS "nvs_storage.c")
    list(APPEND STORAGE_REQUIRES nvs_flash)
endif()

if(CONFIG_ENABLE_STORAGE_SPIFFS)
    list(APPEND STORAGE_REQUIRES spiffs)
endif()

idf_component_register(
    SRCS ${STORAGE_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES ${STORAGE_REQUIRES}
)
