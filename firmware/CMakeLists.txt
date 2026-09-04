# OpenTherm Wi‑Fi MQTT Gateway — ESP-IDF project root
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(otc6_gateway)

# Build gate: app .bin must fit in an OTA slot with 256 KiB headroom (spec FLASH_LAYOUT).
add_custom_target(check_ota_size ALL
    COMMAND ${CMAKE_COMMAND}
        -DOTA_BIN_PATH=${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
        -DOTA_PARTITIONS_CSV=${CMAKE_CURRENT_SOURCE_DIR}/partitions.csv
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_ota_size.cmake
    DEPENDS gen_project_binary
    COMMENT "Checking app size < OTA slot − 256 KiB"
    VERBATIM
)
