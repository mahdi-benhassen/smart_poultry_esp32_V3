# Control Component CMakeLists.txt

idf_component_register(
    SRCS 
        "control_manager.c"
        "pid_controller.c"
        "rules_engine.c"
        "decision_algorithm.c"
        "emergency_protocol.c"
        "energy_optimizer.c"
    INCLUDE_DIRS 
        "include"
    REQUIRES esp32 actuators sensors
)
