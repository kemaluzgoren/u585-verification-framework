set(LPS22HH_DIR ${PLATFORM_DIR}/LPS22HH)

# ---- lps22hh.c: driver logic, lps22hh_reg.h mocked out via CMock ----
# :ignore_arg      lets the test skip comparing the output-buffer pointer.
# :return_thru_ptr lets the test make the mock write a simulated register
#                   value through that same pointer.
add_cmock_mock(
  HEADER  ${LPS22HH_DIR}/lps22hh_reg.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   lps22hh_reg_mock_c
  OUT_H   lps22hh_reg_mock_h
)

add_unity_test(
  NAME         test_lps22hh
  SOURCES      test_lps22hh.c ${lps22hh_reg_mock_c} ${LPS22HH_DIR}/lps22hh.c
  INCLUDE_DIRS ${LPS22HH_DIR} ${CMOCK_MOCK_DIR}
  MOCK_TARGETS generate_lps22hh_reg_mock
  LIBS         cmock_runtime
)
