set(VEML3235_DIR ${PLATFORM_DIR}/VEML3235)

# ---- veml3235_reg.c: byte-order logic, plain Unity with a hand-written fake bus ----
add_unity_test(
  NAME         test_veml3235_reg
  SOURCES      test_veml3235_reg.c ${VEML3235_DIR}/veml3235_reg.c
  INCLUDE_DIRS ${VEML3235_DIR}
)

# ---- veml3235.c: driver logic, veml3235_reg.h mocked out via CMock ----
# :ignore_arg      lets the test skip comparing the output-buffer pointer.
# :return_thru_ptr lets the test make the mock write a simulated register
#                   value through that same pointer.
add_cmock_mock(
  HEADER  ${VEML3235_DIR}/veml3235_reg.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   veml3235_reg_mock_c
  OUT_H   veml3235_reg_mock_h
)

add_unity_test(
  NAME         test_veml3235
  SOURCES      test_veml3235.c ${veml3235_reg_mock_c} ${VEML3235_DIR}/veml3235.c
  INCLUDE_DIRS ${VEML3235_DIR} ${CMOCK_MOCK_DIR}
  MOCK_TARGETS generate_veml3235_reg_mock
  LIBS         cmock_runtime
)
