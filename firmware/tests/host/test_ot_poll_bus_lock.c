#include "unity.h"
#include "ot_poll.h"

#include <pthread.h>
#include <stdatomic.h>

int host_ot_overlap_count(void);
int host_ot_max_in_flight(void);
void host_ot_reset_concurrency_stats(void);

#define N_PER_THREAD 20

static atomic_int s_failures;

static void *exchange_worker(void *arg)
{
    uint8_t id = *(uint8_t *)arg;
    for (int i = 0; i < N_PER_THREAD; i++) {
        ot_exchange_t ex = {
            .data_id = id,
            .request_value = (uint16_t)i,
            .is_write = false,
        };
        if (ot_poll_exchange(&ex) != OT_EXCHANGE_OK) {
            atomic_fetch_add(&s_failures, 1);
        }
    }
    return NULL;
}

void setUp(void)
{
    host_ot_reset_concurrency_stats();
    atomic_store(&s_failures, 0);
    TEST_ASSERT_EQUAL(ESP_OK, ot_poll_init());
}

void tearDown(void) {}

void test_parallel_exchange_serializes_on_bus(void)
{
    pthread_t t1, t2;
    uint8_t id_a = 1;
    uint8_t id_b = 25;

    TEST_ASSERT_EQUAL(0, pthread_create(&t1, NULL, exchange_worker, &id_a));
    TEST_ASSERT_EQUAL(0, pthread_create(&t2, NULL, exchange_worker, &id_b));
    TEST_ASSERT_EQUAL(0, pthread_join(t1, NULL));
    TEST_ASSERT_EQUAL(0, pthread_join(t2, NULL));

    TEST_ASSERT_EQUAL(0, atomic_load(&s_failures));
    TEST_ASSERT_EQUAL_MESSAGE(0, host_ot_overlap_count(),
                              "esp_ot_send_request overlapped — bus mutex missing?");
    TEST_ASSERT_EQUAL(1, host_ot_max_in_flight());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_parallel_exchange_serializes_on_bus);
    return UNITY_END();
}
