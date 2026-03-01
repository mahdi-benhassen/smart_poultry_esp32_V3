# Actuators Component CMakeLists.txt

idf_component_register(
    SRCS 
        "actuator_manager.c"
        "exhaust_fan.c"
        "cooling_pad.c"
        "heater.c"
        "led_light.c"
        "feeder.c"
        "water_pump.c"
        "alarm.c"
    INCLUDE_DIRS 
        "include"
    REQUIRES esp32 driver
)
