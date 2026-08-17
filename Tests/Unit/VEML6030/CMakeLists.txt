set(VEML6030_DIR ${PLATFORM_DIR}/VEML6030)

# ---- veml6030_reg.c: byte-order logic, plain Unity with a hand-written fake bus ----
add_unity_test(
  NAME         test_veml6030_reg
  SOURCES      test_veml6030_reg.c ${VEML6030_DIR}/veml6030_reg.c
  INCLUDE_DIRS ${VEML6030_DIR}
)

# ---- veml6030.c: driver logic, veml6030_reg.h mocked out via CMock ----
# :ignore_arg      lets the test skip comparing the output-buffer pointer.
# :return_thru_ptr lets the test make the mock write a simulated register
#                   value through that same pointer.
add_cmock_mock(
  HEADER  ${VEML6030_DIR}/veml6030_reg.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   veml6030_reg_mock_c
  OUT_H   veml6030_reg_mock_h
)

add_unity_test(
  NAME         test_veml6030
  SOURCES      test_veml6030.c ${veml6030_reg_mock_c} ${VEML6030_DIR}/veml6030.c
  INCLUDE_DIRS ${VEML6030_DIR} ${CMOCK_MOCK_DIR}
  MOCK_TARGETS generate_veml6030_reg_mock
  LIBS         cmock_runtime
)
