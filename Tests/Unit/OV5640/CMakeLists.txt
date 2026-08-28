set(OV5640_DIR ${PLATFORM_DIR}/OV5640)

# ---- ov5640.c: driver logic, ov5640_reg.h mocked out via CMock ----
add_cmock_mock(
  HEADER  ${OV5640_DIR}/ov5640_reg.h
  PLUGINS ignore_arg return_thru_ptr
  OUT_C   ov5640_reg_mock_c
  OUT_H   ov5640_reg_mock_h
)

add_unity_test(
  NAME         test_ov5640
  SOURCES      test_ov5640.c ${ov5640_reg_mock_c} ${OV5640_DIR}/ov5640.c
  # CMAKE_CURRENT_SOURCE_DIR (this dir's fake cmsis_compiler.h) must come
  # before OV5640_DIR so ov5640_reg.h's #include <cmsis_compiler.h>
  # resolves there instead of pulling in real ARM Cortex-M intrinsics.
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${OV5640_DIR} ${CMOCK_MOCK_DIR}
  MOCK_TARGETS generate_ov5640_reg_mock
  LIBS         cmock_runtime
)
