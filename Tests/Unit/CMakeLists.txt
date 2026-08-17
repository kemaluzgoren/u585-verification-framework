cmake_minimum_required(VERSION 3.22)
project(u585_verification_framework_unit_tests C)

#
# Host-native unit test project for the hardware-independent driver logic
# under Platform/. This is a SEPARATE build from the root CMakeLists.txt:
# the root project cross-compiles for the STM32 target (arm-none-eabi-gcc),
# this one compiles with the host compiler and runs directly on the PC.
#
# Usage:
#   cmake -S Tests/Unit -B Tests/Unit/build -G "MinGW Makefiles"
#   cmake --build Tests/Unit/build
#   ctest --test-dir Tests/Unit/build --output-on-failure
#
# Each driver module under test gets its own subfolder here (mirroring
# Platform/<Module>/), with its own small CMakeLists.txt that calls the
# add_cmock_mock()/add_unity_test() helpers defined below. Add a new
# module by creating the subfolder and appending an add_subdirectory()
# call at the bottom of this file.
#

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  unity
  GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
  GIT_TAG        v2.6.0
)
FetchContent_Declare(
  cmock
  GIT_REPOSITORY https://github.com/ThrowTheSwitch/CMock.git
  GIT_TAG        v2.6.0
)

# Fetch sources only: neither repo's own build system is used here, since
# Unity ships no top-level CMakeLists.txt and CMock's pulls in tooling
# (Meson/Ruby gem tasks) this project does not need. Unity's runner and
# CMock's mock-support runtime are each compiled directly below instead.
FetchContent_GetProperties(unity)
if(NOT unity_POPULATED)
  FetchContent_Populate(unity)
endif()

FetchContent_GetProperties(cmock)
if(NOT cmock_POPULATED)
  FetchContent_Populate(cmock)
endif()

find_program(RUBY_EXECUTABLE ruby HINTS "C:/Ruby33-x64/bin")
if(NOT RUBY_EXECUTABLE)
  message(FATAL_ERROR "Ruby is required to run CMock's mock generator (https://rubyinstaller.org)")
endif()

set(PLATFORM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../Platform)
set(CMOCK_MOCK_DIR ${CMAKE_CURRENT_BINARY_DIR}/mocks)
file(MAKE_DIRECTORY ${CMOCK_MOCK_DIR})

# ---- Unity test runner ----
add_library(unity STATIC ${unity_SOURCE_DIR}/src/unity.c)
target_include_directories(unity PUBLIC ${unity_SOURCE_DIR}/src)

# ---- CMock mock-support runtime (needed by any CMock-generated mock) ----
add_library(cmock_runtime STATIC ${cmock_SOURCE_DIR}/src/cmock.c)
target_include_directories(cmock_runtime PUBLIC ${cmock_SOURCE_DIR}/src)
target_link_libraries(cmock_runtime PUBLIC unity)

enable_testing()

# Generates a CMock mock for HEADER (an absolute path). Sets OUT_C/OUT_H in
# the caller's scope to the generated Mock<name>.c/.h paths.
#
#   add_cmock_mock(
#     HEADER    ${PLATFORM_DIR}/VEML6030/veml6030_reg.h
#     PLUGINS   ignore_arg return_thru_ptr   # optional, plain CMake list
#     OUT_C     mock_c
#     OUT_H     mock_h
#   )
#
# PLUGINS is written into a small per-header YAML config file rather than
# passed as a CMock --plugins= CLI flag: CMock expects that flag's value to
# be semicolon-joined, which collides with CMake's own list separator and
# gets silently mangled once it has passed through a function call.
function(add_cmock_mock)
  set(oneValueArgs HEADER OUT_C OUT_H)
  set(multiValueArgs PLUGINS)
  cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  get_filename_component(header_name ${ARG_HEADER} NAME_WE)
  set(mock_c ${CMOCK_MOCK_DIR}/Mock${header_name}.c)
  set(mock_h ${CMOCK_MOCK_DIR}/Mock${header_name}.h)
  set(config_yml ${CMAKE_CURRENT_BINARY_DIR}/cmock_${header_name}.yml)

  set(plugins_yaml "  :plugins: []\n")
  if(ARG_PLUGINS)
    set(plugins_yaml "  :plugins:\n")
    foreach(plugin ${ARG_PLUGINS})
      string(APPEND plugins_yaml "    - :${plugin}\n")
    endforeach()
  endif()
  file(WRITE ${config_yml} ":cmock:\n  :mock_path: ${CMOCK_MOCK_DIR}\n${plugins_yaml}")

  add_custom_command(
    OUTPUT  ${mock_c} ${mock_h}
    COMMAND ${RUBY_EXECUTABLE} ${cmock_SOURCE_DIR}/lib/cmock.rb -o${config_yml} ${ARG_HEADER}
    DEPENDS ${ARG_HEADER} ${config_yml}
    COMMENT "CMock: generating mock for ${header_name}.h"
    VERBATIM
  )
  add_custom_target(generate_${header_name}_mock DEPENDS ${mock_c} ${mock_h})

  set(${ARG_OUT_C} ${mock_c} PARENT_SCOPE)
  set(${ARG_OUT_H} ${mock_h} PARENT_SCOPE)
endfunction()

# Defines a Unity-based test executable and registers it with ctest.
#
#   add_unity_test(
#     NAME          test_veml6030
#     SOURCES       test_veml6030.c ${mock_c} ${PLATFORM_DIR}/VEML6030/veml6030.c
#     INCLUDE_DIRS  ${PLATFORM_DIR}/VEML6030 ${CMOCK_MOCK_DIR}
#     MOCK_TARGETS  generate_veml6030_reg_mock  # optional
#     LIBS          cmock_runtime               # optional, in addition to unity
#   )
function(add_unity_test)
  set(oneValueArgs NAME)
  set(multiValueArgs SOURCES INCLUDE_DIRS MOCK_TARGETS LIBS)
  cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_executable(${ARG_NAME} ${ARG_SOURCES})
  if(ARG_INCLUDE_DIRS)
    target_include_directories(${ARG_NAME} PRIVATE ${ARG_INCLUDE_DIRS})
  endif()
  if(ARG_MOCK_TARGETS)
    add_dependencies(${ARG_NAME} ${ARG_MOCK_TARGETS})
  endif()
  target_link_libraries(${ARG_NAME} PRIVATE unity ${ARG_LIBS})
  add_test(NAME ${ARG_NAME} COMMAND ${ARG_NAME})
endfunction()

add_subdirectory(VEML6030)
