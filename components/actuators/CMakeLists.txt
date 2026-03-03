# Actuators Component CMakeLists.txt
# Smart Poultry V3 - Modular Architecture

# Build source list based on enabled actuators
set(ACTUATOR_SRCS "actuator_manager.c")

#ifdef CONFIG_ENABLE_ACTUATOR_EXHAUST_FAN
if(CONFIG_ENABLE_ACTUATOR_EXHAUST_FAN)
    list(APPEND ACTUATOR_SRCS "exhaust_fan.c")
endif()
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_COOLING_PAD
if(CONFIG_ENABLE_ACTUATOR_COOLING_PAD)
    list(APPEND ACTUATOR_SRCS "cooling_pad.c")
endif()
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_HEATER
if(CONFIG_ENABLE_ACTUATOR_HEATER)
    list(APPEND ACTUATOR_SRCS "heater.c")
endif()
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_LED_LIGHT
if(CONFIG_ENABLE_ACTUATOR_LED_LIGHT)
    list(APPEND ACTUATOR_SRCS "led_light.c")
endif()
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_FEEDER
if(CONFIG_ENABLE_ACTUATOR_FEEDER)
    list(APPEND ACTUATOR_SRCS "feeder.c")
endif()
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_WATER_PUMP
if(CONFIG_ENABLE_ACTUATOR_WATER_PUMP)
    list(APPEND ACTUATOR_SRCS "water_pump.c")
endif()
#endif

#ifdef CONFIG_ENABLE_ACTUATOR_ALARM
if(CONFIG_ENABLE_ACTUATOR_ALARM)
    list(APPEND ACTUATOR_SRCS "alarm.c")
endif()
#endif

idf_component_register(
    SRCS ${ACTUATOR_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES driver
)
