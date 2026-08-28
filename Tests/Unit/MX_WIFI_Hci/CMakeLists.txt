set(MX_WIFI_DIR ${PLATFORM_DIR}/MX_WIFI)

# CMAKE_CURRENT_SOURCE_DIR (this dir's mx_wifi_conf.h/main.h stand-ins,
# same as Tests/Unit/MX_WIFI_Slip's - see that directory's CMakeLists.txt
# comment) must come before MX_WIFI_DIR. mx_rtos_abs.c is compiled here
# for its bare-metal noos_fifo_init()/_deinit() (used by
# mx_wifi_hci_init()/_deinit()) - the real, vendor-shipped no-OS
# implementation, not a mock.
add_unity_test(
  NAME         test_mx_wifi_hci
  SOURCES      test_mx_wifi_hci.c ${MX_WIFI_DIR}/core/mx_wifi_hci.c ${MX_WIFI_DIR}/core/mx_rtos_abs.c
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${MX_WIFI_DIR} ${MX_WIFI_DIR}/core
)
