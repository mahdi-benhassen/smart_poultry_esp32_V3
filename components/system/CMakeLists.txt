# System Component CMakeLists.txt

idf_component_register(
    SRCS "task_watchdog.c"
    INCLUDE_DIRS "include"
    REQUIRES freertos
)