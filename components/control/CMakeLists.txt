# Control Component CMakeLists.txt
# Smart Poultry V3 - Modular Architecture

set(CONTROL_SRCS "control_manager.c")

if(CONFIG_ENABLE_CONTROL_PID)
    list(APPEND CONTROL_SRCS "pid_controller.c")
endif()

if(CONFIG_ENABLE_CONTROL_RULES_ENGINE)
    list(APPEND CONTROL_SRCS "rules_engine.c")
endif()

list(APPEND CONTROL_SRCS "decision_algorithm.c")

if(CONFIG_ENABLE_CONTROL_EMERGENCY_PROTOCOL)
    list(APPEND CONTROL_SRCS "emergency_protocol.c")
endif()

if(CONFIG_ENABLE_CONTROL_ENERGY_OPTIMIZER)
    list(APPEND CONTROL_SRCS "energy_optimizer.c")
endif()

idf_component_register(
    SRCS ${CONTROL_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES actuators sensors
)
