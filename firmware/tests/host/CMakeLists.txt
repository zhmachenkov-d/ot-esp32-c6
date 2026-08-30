# Host-side Unity tests (no target flash required)
#
#   ./run.sh
#   cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure

cmake_minimum_required(VERSION 3.16)
project(otc6_host_tests C)

set(CMAKE_C_STANDARD 11)

set(UNITY_DIR "$ENV{IDF_PATH}/components/unity/unity")
if(NOT EXISTS "${UNITY_DIR}/src/unity.c")
  message(FATAL_ERROR "Unity not found at ${UNITY_DIR}; set IDF_PATH")
endif()

set(MAIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../main")

add_compile_definitions(HOST_TEST=1)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs
  ${MAIN_DIR}
  ${UNITY_DIR}/src
)

add_library(unity STATIC ${UNITY_DIR}/src/unity.c)

add_library(otc6_host STATIC
  ${MAIN_DIR}/ot_codec.c
  ${MAIN_DIR}/failsafe.c
  ${MAIN_DIR}/ot_catalog.c
  ${MAIN_DIR}/provision_validate.c
  ${CMAKE_CURRENT_SOURCE_DIR}/host_catalog_nvs_stub.c
  ${CMAKE_CURRENT_SOURCE_DIR}/host_ot_poll_stub.c
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs/mqtt_ha_stub.c
  ${CMAKE_CURRENT_SOURCE_DIR}/host_discovery_builders.c
  ${CMAKE_CURRENT_SOURCE_DIR}/host_commands_core.c
)

target_link_libraries(otc6_host PUBLIC m)

function(add_host_test name src)
  add_executable(${name} ${src})
  target_link_libraries(${name} otc6_host unity m)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

enable_testing()

add_host_test(test_ot_codec test_ot_codec.c)
add_host_test(test_ot_catalog test_ot_catalog.c)
add_host_test(test_mqtt_discovery test_mqtt_discovery.c)
add_host_test(test_status_projections test_status_projections.c)
add_host_test(test_provision_validate test_provision_validate.c)
add_host_test(test_setpoint_bounds test_setpoint_bounds.c)
add_host_test(test_mqtt_commands test_mqtt_commands.c)
add_host_test(test_failsafe test_failsafe.c)

# Links real ot_poll.c (bus mutex) with host FreeRTOS/OpenTherm stubs — not the poll stub.
add_executable(test_ot_poll_bus_lock
  test_ot_poll_bus_lock.c
  ${MAIN_DIR}/ot_poll.c
  ${MAIN_DIR}/ot_codec.c
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs/mqtt_ha_stub.c
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs/freertos_host_stub.c
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs/opentherm_host_stub.c
)
target_link_libraries(test_ot_poll_bus_lock unity m pthread)
add_test(NAME test_ot_poll_bus_lock COMMAND test_ot_poll_bus_lock)
