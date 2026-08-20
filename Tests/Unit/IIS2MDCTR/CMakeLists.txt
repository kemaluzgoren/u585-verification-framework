set(IIS2MDCTR_DIR ${PLATFORM_DIR}/IIS2MDCTR)

# ---- iis2mdc.c: driver logic, iis2mdc_reg.h mocked out via CMock ----
add_cmock_mock(
  HEADER  ${IIS2MDCTR_DIR}/iis2mdc_reg.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   iis2mdc_reg_mock_c
  OUT_H   iis2mdc_reg_mock_h
)

add_unity_test(
  NAME         test_iis2mdc
  SOURCES      test_iis2mdc.c ${iis2mdc_reg_mock_c} ${IIS2MDCTR_DIR}/iis2mdc.c
  INCLUDE_DIRS ${IIS2MDCTR_DIR} ${CMOCK_MOCK_DIR}
  MOCK_TARGETS generate_iis2mdc_reg_mock
  LIBS         cmock_runtime
)
