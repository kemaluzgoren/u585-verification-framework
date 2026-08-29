set(NETWORK_DIR ${APPLICATION_DIR}/Network)

# http_pages.c's #include "network_service.h" is a quote-include resolved
# relative to http_pages.c's own directory first - the real
# network_service.h (right next to it) is what gets found regardless of
# -I order, so it is compiled for real here (see this directory's
# tx_api.h for why that needs faking instead).
add_unity_test(
  NAME         test_http_pages
  SOURCES      test_http_pages.c ${NETWORK_DIR}/http_pages.c
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${NETWORK_DIR}
)
