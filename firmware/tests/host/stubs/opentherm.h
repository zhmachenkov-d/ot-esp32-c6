#pragma once

#include "esp_err.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    OT_STATUS_NONE = 0,
    OT_STATUS_SUCCESS,
    OT_STATUS_INVALID,
    OT_STATUS_TIMEOUT
} open_therm_response_status_t;

typedef enum {
    OT_READ_DATA = 0,
    OT_WRITE_DATA = 1,
    OT_READ_ACK = 4,
    OT_WRITE_ACK = 5,
    OT_DATA_INVALID = 6,
    OT_UNKNOWN_DATA_ID = 7
} open_therm_message_type_t;

typedef uint8_t open_therm_message_id_t;

esp_err_t esp_ot_init(int in_pin, int out_pin, bool slave, void *cb);
unsigned long esp_ot_build_request(open_therm_message_type_t type,
                                   open_therm_message_id_t id, unsigned int data);
unsigned long esp_ot_send_request(unsigned long request);
open_therm_response_status_t esp_ot_get_last_response_status(void);
open_therm_message_type_t esp_ot_get_message_type(unsigned long message);
uint16_t esp_ot_get_uint(const unsigned long response);
