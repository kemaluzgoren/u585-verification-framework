set(HTS221_DIR ${PLATFORM_DIR}/HTS221)

# ---- hts221.c: driver logic, hts221_reg.h mocked out via CMock ----
# :ignore_arg      lets the test skip comparing the output-buffer pointer.
# :return_thru_ptr lets the test make the mock write a simulated register
#                   value through that same pointer.
add_cmock_mock(
  HEADER  ${HTS221_DIR}/hts221_reg.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   hts221_reg_mock_c
  OUT_H   hts221_reg_mock_h
)

add_unity_test(
  NAME         test_hts221
  SOURCES      test_hts221.c ${hts221_reg_mock_c} ${HTS221_DIR}/hts221.c
  INCLUDE_DIRS ${HTS221_DIR} ${CMOCK_MOCK_DIR}
  MOCK_TARGETS generate_hts221_reg_mock
  LIBS         cmock_runtime
)
