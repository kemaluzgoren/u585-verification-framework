set(ISM330DHCX_DIR ${PLATFORM_DIR}/ISM330DHCX)

# ---- ism330dhcx.c: driver logic, ism330dhcx_reg.h mocked out via CMock ----
add_cmock_mock(
  HEADER  ${ISM330DHCX_DIR}/ism330dhcx_reg.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   ism330dhcx_reg_mock_c
  OUT_H   ism330dhcx_reg_mock_h
)

add_unity_test(
  NAME         test_ism330dhcx
  SOURCES      test_ism330dhcx.c ${ism330dhcx_reg_mock_c} ${ISM330DHCX_DIR}/ism330dhcx.c
  INCLUDE_DIRS ${ISM330DHCX_DIR} ${CMOCK_MOCK_DIR}
  MOCK_TARGETS generate_ism330dhcx_reg_mock
  LIBS         cmock_runtime
)
