# For ESP-IDF v5.x+ (requires CMake 3.16 or higher)
cmake_minimum_required(VERSION 3.16)

# Set project name
set(PROJECT_NAME "smart_poultry")

# Include ESP-IDF
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Project sources
project(${PROJECT_NAME})
