set(MX_CHIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../Platform/MX_CHIP)

# Real ThreadX common/src object-management source this directory's test
# needs, compiled straight from the vendored Middlewares/ST/threadx tree -
# no mocks, no separate host port download. See threadx_host_glue.c's
# header comment for exactly which files are (and are not) safe to link
# this way and why.
set(THREADX_REAL_SOURCES
  ${THREADX_COMMON_SRC_DIR}/tx_byte_pool_create.c
  ${THREADX_COMMON_SRC_DIR}/tx_byte_allocate.c
  ${THREADX_COMMON_SRC_DIR}/tx_byte_release.c
  ${THREADX_COMMON_SRC_DIR}/tx_byte_pool_delete.c
  ${THREADX_COMMON_SRC_DIR}/tx_byte_pool_prioritize.c
  ${THREADX_COMMON_SRC_DIR}/tx_byte_pool_cleanup.c
  ${THREADX_COMMON_SRC_DIR}/tx_byte_pool_search.c
  ${THREADX_COMMON_SRC_DIR}/tx_queue_create.c
  ${THREADX_COMMON_SRC_DIR}/tx_queue_delete.c
  ${THREADX_COMMON_SRC_DIR}/tx_queue_send.c
  ${THREADX_COMMON_SRC_DIR}/tx_queue_receive.c
  ${THREADX_COMMON_SRC_DIR}/tx_queue_cleanup.c
  ${THREADX_COMMON_SRC_DIR}/tx_queue_prioritize.c
)

# CMAKE_CURRENT_SOURCE_DIR (this dir's nx_api.h/mx_wifi.h stand-ins) must
# come before MX_CHIP_DIR so mx_wifi_azure_rtos.c's #include "mx_wifi.h"
# resolves to the local fake instead of the real one; THREADX_REAL_INC_DIRS
# is the real Middlewares tx_api.h/tx_port.h - #include "tx_api.h" resolves
# there since no fake tx_api.h exists in this directory (unlike
# ../MX_WIFI_AzureRtos, which mocks it instead).
add_unity_test(
  NAME         test_mx_wifi_threadx
  SOURCES      test_mx_wifi_threadx.c threadx_host_glue.c nx_stubs.c
               ${MX_CHIP_DIR}/mx_wifi_azure_rtos.c ${THREADX_REAL_SOURCES}
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${MX_CHIP_DIR} ${THREADX_REAL_INC_DIRS}
)

target_compile_definitions(test_mx_wifi_threadx PRIVATE
  # Routes tx_port.h's TX_DISABLE/TX_RESTORE through
  # _tx_thread_interrupt_disable()/_tx_thread_interrupt_restore()
  # (threadx_host_glue.c) instead of Cortex-M PRIMASK/CPSID inline
  # assembly, which the host compiler cannot assemble.
  TX_DISABLE_INLINE
  # Maps tx_* API calls straight to their _tx_* core implementations,
  # skipping the _txe_*/_txr_* error-checking front-ends - those add
  # parameter validation on top of the same core logic this test already
  # exercises, and skipping them avoids pulling in _tx_thread_system_state
  # gating meant for catching calls made from an interrupt context, which
  # is meaningless for a single host test process.
  TX_DISABLE_ERROR_CHECKING
  # Pulls in this directory's tx_user.h, which widens ALIGN_TYPE to a real
  # pointer-sized type for this 64-bit host build - see tx_user.h's header
  # comment for why that is necessary (and the intended fix) here.
  TX_INCLUDE_USER_DEFINE_FILE
)
