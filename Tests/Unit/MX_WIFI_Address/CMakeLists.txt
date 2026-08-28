set(MX_WIFI_DIR ${PLATFORM_DIR}/MX_WIFI)

# Pure parsing/formatting, no mocking needed - CMAKE_CURRENT_SOURCE_DIR
# (this directory's own mx_wifi.h) MUST come before MX_WIFI_DIR/core so
# mx_address.c's #include "mx_wifi.h" resolves to the local stand-in
# instead of the real header (see that file's header comment for why).
add_unity_test(
  NAME         test_mx_address
  SOURCES      test_mx_address.c ${MX_WIFI_DIR}/core/mx_address.c
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${MX_WIFI_DIR}/core
)
