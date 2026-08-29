set(SENSOR_SERVICE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../Application/Sensors/Sensor_Service)

# Real ThreadX mutex object-management source this directory's test needs
# - see threadx_host_glue.c's header comment for the general approach
# (same one Tests/Integration/MX_WIFI_ThreadX uses, trimmed to just the
# mutex family since that is all sensor_service.c touches).
set(THREADX_REAL_SOURCES
  ${THREADX_COMMON_SRC_DIR}/tx_mutex_create.c
  ${THREADX_COMMON_SRC_DIR}/tx_mutex_get.c
  ${THREADX_COMMON_SRC_DIR}/tx_mutex_put.c
  ${THREADX_COMMON_SRC_DIR}/tx_mutex_delete.c
  ${THREADX_COMMON_SRC_DIR}/tx_mutex_prioritize.c
  ${THREADX_COMMON_SRC_DIR}/tx_mutex_cleanup.c
  ${THREADX_COMMON_SRC_DIR}/tx_mutex_priority_change.c
)

# CMAKE_CURRENT_SOURCE_DIR (this dir's light_sensor.h/humidity_sensor.h/
# pressure_sensor.h/magneto_sensor.h/imu_sensor.h stand-ins) must come
# before SENSOR_SERVICE_DIR so sensor_service.c's #include "light_sensor.h"
# (etc.) resolves to the local fakes instead of the real headers - those
# pull in i2c_bus.h -> stm32u5xx_hal.h (ARM-target only). THREADX_REAL_INC_DIRS
# is the real Middlewares tx_api.h/tx_port.h - #include "tx_api.h" resolves
# there since no fake tx_api.h exists in this directory.
add_cmock_mock(
  HEADER  ${CMAKE_CURRENT_SOURCE_DIR}/light_sensor.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   light_sensor_mock_c
  OUT_H   light_sensor_mock_h
)
add_cmock_mock(
  HEADER  ${CMAKE_CURRENT_SOURCE_DIR}/humidity_sensor.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   humidity_sensor_mock_c
  OUT_H   humidity_sensor_mock_h
)
add_cmock_mock(
  HEADER  ${CMAKE_CURRENT_SOURCE_DIR}/pressure_sensor.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   pressure_sensor_mock_c
  OUT_H   pressure_sensor_mock_h
)
add_cmock_mock(
  HEADER  ${CMAKE_CURRENT_SOURCE_DIR}/magneto_sensor.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   magneto_sensor_mock_c
  OUT_H   magneto_sensor_mock_h
)
add_cmock_mock(
  HEADER  ${CMAKE_CURRENT_SOURCE_DIR}/imu_sensor.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   imu_sensor_mock_c
  OUT_H   imu_sensor_mock_h
)

add_unity_test(
  NAME         test_sensor_service
  SOURCES      test_sensor_service.c threadx_host_glue.c
               ${light_sensor_mock_c} ${humidity_sensor_mock_c} ${pressure_sensor_mock_c}
               ${magneto_sensor_mock_c} ${imu_sensor_mock_c}
               ${SENSOR_SERVICE_DIR}/sensor_service.c ${THREADX_REAL_SOURCES}
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${SENSOR_SERVICE_DIR} ${THREADX_REAL_INC_DIRS} ${CMOCK_MOCK_DIR}
  MOCK_TARGETS generate_light_sensor_mock generate_humidity_sensor_mock generate_pressure_sensor_mock
               generate_magneto_sensor_mock generate_imu_sensor_mock
  LIBS         cmock_runtime
)

target_compile_definitions(test_sensor_service PRIVATE
  # See Tests/Integration/MX_WIFI_ThreadX/CMakeLists.txt's comments on the
  # same two defines - identical reasoning here.
  TX_DISABLE_INLINE
  TX_DISABLE_ERROR_CHECKING
)
