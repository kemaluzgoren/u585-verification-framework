set(MX_WIFI_DIR ${PLATFORM_DIR}/MX_WIFI)

# Pure algorithm, no HW/RTOS dependency to mock out.
add_unity_test(
  NAME         test_checksumutils
  SOURCES      test_checksumutils.c ${MX_WIFI_DIR}/core/checksumutils.c
  INCLUDE_DIRS ${MX_WIFI_DIR}/core
)
