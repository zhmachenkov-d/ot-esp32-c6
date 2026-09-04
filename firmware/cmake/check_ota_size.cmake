# Fail the build if the app binary exceeds OTA slot size minus 256 KiB.
# Slot size is parsed from partitions.csv ota_0 Size column.
if(NOT DEFINED OTA_BIN_PATH)
  message(FATAL_ERROR "OTA_BIN_PATH not set")
endif()
if(NOT DEFINED OTA_PARTITIONS_CSV)
  message(FATAL_ERROR "OTA_PARTITIONS_CSV not set")
endif()
if(NOT EXISTS "${OTA_BIN_PATH}")
  message(FATAL_ERROR "App binary not found: ${OTA_BIN_PATH}")
endif()
if(NOT EXISTS "${OTA_PARTITIONS_CSV}")
  message(FATAL_ERROR "partitions.csv not found: ${OTA_PARTITIONS_CSV}")
endif()

file(READ "${OTA_PARTITIONS_CSV}" _ota_pt_raw)
string(REGEX MATCH "ota_0[ \t]*,[ \t]*app[ \t]*,[ \t]*ota_0[ \t]*,[ \t]*0x[0-9A-Fa-f]+[ \t]*,[ \t]*(0x[0-9A-Fa-f]+)"
       _ota_pt_match "${_ota_pt_raw}")
if(NOT CMAKE_MATCH_1)
  message(FATAL_ERROR "Could not parse ota_0 size from ${OTA_PARTITIONS_CSV}")
endif()
math(EXPR OTA_SLOT_BYTES "${CMAKE_MATCH_1}")
math(EXPR OTA_MAX_BYTES "${OTA_SLOT_BYTES} - 262144") # slot − 256 KiB

file(SIZE "${OTA_BIN_PATH}" APP_BYTES)
if(APP_BYTES GREATER_EQUAL OTA_MAX_BYTES)
  message(FATAL_ERROR
    "App image ${APP_BYTES} bytes >= OTA limit ${OTA_MAX_BYTES} (slot ${OTA_SLOT_BYTES} − 256 KiB)")
endif()
message(STATUS "OTA size gate OK: ${APP_BYTES} < ${OTA_MAX_BYTES} bytes (slot ${OTA_SLOT_BYTES} from partitions.csv)")
