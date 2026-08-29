cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME u585-verification-framework)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Application/System/system.c
    Application/I2C_Bus/i2c_bus.c
    Application/Sensors/Light_Sensor/light_sensor.c
    Application/Sensors/Humidity_Sensor/humidity_sensor.c
    Application/Sensors/Pressure_Sensor/pressure_sensor.c
    Application/Sensors/Magneto_Sensor/magneto_sensor.c
    Application/Sensors/IMU_Sensor/imu_sensor.c
    Application/Sensors/Sensor_Service/sensor_service.c
    Application/Network/network_service.c
    Application/Network/http_responses.c
    Application/Network/http_pages.c
    Application/Camera/camera_service.c
    Platform/OV5640/ov5640.c
    Platform/OV5640/ov5640_reg.c
    Platform/VEML6030/veml6030.c
    Platform/VEML6030/veml6030_reg.c
    Platform/VEML3235/veml3235.c
    Platform/VEML3235/veml3235_reg.c
    Platform/HTS221/hts221.c
    Platform/HTS221/hts221_reg.c
    Platform/LPS22HH/lps22hh.c
    Platform/LPS22HH/lps22hh_reg.c
    Platform/IIS2MDCTR/iis2mdc.c
    Platform/IIS2MDCTR/iis2mdc_reg.c
    Platform/ISM330DHCX/ism330dhcx.c
    Platform/ISM330DHCX/ism330dhcx_reg.c
    Application/WiFi_Bus/wifi_bus.c
    Platform/MX_WIFI/mx_wifi.c
    Platform/MX_WIFI/core/checksumutils.c
    Platform/MX_WIFI/core/mx_address.c
    Platform/MX_WIFI/core/mx_wifi_hci.c
    Platform/MX_WIFI/core/mx_wifi_ipc.c
    Platform/MX_WIFI/core/mx_wifi_slip.c
    Platform/MX_CHIP/mx_wifi_azure_rtos.c
    Platform/MX_CHIP/nx_driver_emw3080.c
    NetXDuo/App/filex_stub.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Application/System
    Application/I2C_Bus
    Application/Sensors/Light_Sensor
    Application/Sensors/Humidity_Sensor
    Application/Sensors/Pressure_Sensor
    Application/Sensors/Magneto_Sensor
    Application/Sensors/IMU_Sensor
    Application/Sensors/Sensor_Service
    Application/Network
    Application/Camera
    Platform/OV5640
    Platform/VEML6030
    Platform/VEML3235
    Platform/HTS221
    Platform/LPS22HH
    Platform/IIS2MDCTR
    Platform/ISM330DHCX
    Application/WiFi_Bus
    Platform/MX_WIFI
    Platform/MX_WIFI/core
    Platform/MX_WIFI/io_pattern
    Platform/MX_CHIP
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    USE_THREADX
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
