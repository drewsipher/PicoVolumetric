cmake_minimum_required(VERSION 3.0.0)

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(picoVolumetric VERSION 0.1.0)

include(CTest)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

enable_testing()

add_executable(picoVolumetric main.cpp)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(picoVolumetric pico_stdlib hardware_i2c)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(picoVolumetric)

# Enable Serial for the project 1 is on 0 is off
pico_enable_stdio_usb(picoVolumetric 1)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
