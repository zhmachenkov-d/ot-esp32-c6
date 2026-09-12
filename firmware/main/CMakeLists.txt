idf_component_register(
    SRCS
        "main.c"
        "nvs_store.c"
        "ot_codec.c"
        "ot_poll.c"
        "ot_catalog.c"
        "mqtt_ha.c"
        "mqtt_discovery.c"
        "mqtt_commands.c"
        "failsafe.c"
        "provision_softap.c"
        "provision_validate.c"
        "ota_update.c"
        "status_led.c"
        "status_led_ladder.c"
    INCLUDE_DIRS "."
    REQUIRES
        nvs_flash
        esp_wifi
        esp_event
        esp_netif
        esp_http_server
        mqtt
        json
        driver
        esp_timer
        opentherm
        dns_server
        mbedtls
        app_update
        esp_https_ota
        esp_http_client
        led_strip
)

# Optional CI inject: idf.py -DAPP_FW_VERSION=X.Y.Z build (tag without leading v).
if(DEFINED APP_FW_VERSION AND NOT "${APP_FW_VERSION}" STREQUAL "")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE "APP_FW_VERSION=\"${APP_FW_VERSION}\"")
endif()
