# For ESP-IDF v4.4+
cmake_minimum_required(VERSION 3.5)

# Set project name
set(PROJECT_NAME "smart_poultry")

# Include ESP-IDF
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Project sources
project(${PROJECT_NAME})
