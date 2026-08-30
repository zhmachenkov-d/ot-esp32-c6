#include "opentherm.h"

#include <stdatomic.h>
#include <unistd.h>

static atomic_int s_in_flight;
static atomic_int s_max_in_flight;
static atomic_int s_overlap_count;
static open_therm_response_status_t s_last_status = OT_STATUS_SUCCESS;

int host_ot_overlap_count(void)
{
    return atomic_load(&s_overlap_count);
}

int host_ot_max_in_flight(void)
{
    return atomic_load(&s_max_in_flight);
}

void host_ot_reset_concurrency_stats(void)
{
    atomic_store(&s_in_flight, 0);
    atomic_store(&s_max_in_flight, 0);
    atomic_store(&s_overlap_count, 0);
}

esp_err_t esp_ot_init(int in_pin, int out_pin, bool slave, void *cb)
{
    (void)in_pin;
    (void)out_pin;
    (void)slave;
    (void)cb;
    return ESP_OK;
}

unsigned long esp_ot_build_request(open_therm_message_type_t type,
                                   open_therm_message_id_t id, unsigned int data)
{
    return ((unsigned long)type << 28) | ((unsigned long)id << 16) | (data & 0xffffu);
}

unsigned long esp_ot_send_request(unsigned long request)
{
    int cur = atomic_fetch_add(&s_in_flight, 1) + 1;
    if (cur > 1) {
        atomic_fetch_add(&s_overlap_count, 1);
    }
    int max = atomic_load(&s_max_in_flight);
    while (cur > max && !atomic_compare_exchange_weak(&s_max_in_flight, &max, cur)) {
        max = atomic_load(&s_max_in_flight);
    }
    /* Widen the race window so a missing bus lock fails this test. */
    usleep(2000);
    atomic_fetch_sub(&s_in_flight, 1);
    s_last_status = OT_STATUS_SUCCESS;
    return request;
}

open_therm_response_status_t esp_ot_get_last_response_status(void)
{
    return s_last_status;
}

open_therm_message_type_t esp_ot_get_message_type(unsigned long message)
{
    (void)message;
    return OT_READ_ACK;
}

uint16_t esp_ot_get_uint(const unsigned long response)
{
    return (uint16_t)(response & 0xffffu);
}
