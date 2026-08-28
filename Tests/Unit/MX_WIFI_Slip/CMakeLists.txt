set(MX_WIFI_DIR ${PLATFORM_DIR}/MX_WIFI)

# CMAKE_CURRENT_SOURCE_DIR (this dir's mx_wifi_conf.h/main.h stand-ins)
# must come before MX_WIFI_DIR so mx_wifi_slip.c's transitive
# #include "mx_wifi_conf.h" / "main.h" resolve to the local fakes - see
# mx_wifi_conf.h's header comment for why. No CMock mock needed: this
# selects the real, vendor-shipped bare-metal (non-RTOS) implementation
# instead of mocking anything.
add_unity_test(
  NAME         test_mx_wifi_slip
  SOURCES      test_mx_wifi_slip.c ${MX_WIFI_DIR}/core/mx_wifi_slip.c
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${MX_WIFI_DIR} ${MX_WIFI_DIR}/core
)
