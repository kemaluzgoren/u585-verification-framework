set(MX_CHIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../Platform/MX_CHIP)

# CMAKE_CURRENT_SOURCE_DIR (this dir's tx_api.h/nx_api.h/mx_wifi.h
# stand-ins) must come before MX_CHIP_DIR so mx_wifi_azure_rtos.c's
# #include "mx_wifi.h"/"tx_api.h"/"nx_api.h" resolve to the local fakes
# instead of the real headers (see each fake's header comment for why).
add_cmock_mock(
  HEADER  ${CMAKE_CURRENT_SOURCE_DIR}/tx_api.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   tx_api_mock_c
  OUT_H   tx_api_mock_h
)

add_cmock_mock(
  HEADER  ${CMAKE_CURRENT_SOURCE_DIR}/nx_api.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   nx_api_mock_c
  OUT_H   nx_api_mock_h
)

add_unity_test(
  NAME         test_mx_wifi_azure_rtos
  SOURCES      test_mx_wifi_azure_rtos.c ${tx_api_mock_c} ${nx_api_mock_c} ${MX_CHIP_DIR}/mx_wifi_azure_rtos.c
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${MX_CHIP_DIR} ${CMOCK_MOCK_DIR}
  MOCK_TARGETS generate_tx_api_mock generate_nx_api_mock
  LIBS         cmock_runtime
)
