#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct host_queue {
    UBaseType_t len;
    UBaseType_t item_size;
    UBaseType_t count;
    uint8_t *buf;
};

void vTaskDelay(TickType_t ticks)
{
    if (ticks > 0) {
        usleep((useconds_t)ticks * 1000u);
    }
}

TickType_t xTaskGetTickCount(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (TickType_t)(ts.tv_sec * 1000u + ts.tv_nsec / 1000000u);
}

BaseType_t xTaskCreate(TaskFunction_t fn, const char *name, uint32_t stack,
                       void *arg, UBaseType_t prio, TaskHandle_t *out)
{
    (void)fn;
    (void)name;
    (void)stack;
    (void)arg;
    (void)prio;
    if (out) {
        *out = (TaskHandle_t)1;
    }
    return pdPASS;
}

QueueHandle_t xQueueCreate(UBaseType_t len, UBaseType_t item_size)
{
    struct host_queue *q = calloc(1, sizeof(*q));
    if (!q) {
        return NULL;
    }
    q->len = len;
    q->item_size = item_size;
    q->buf = calloc(len, item_size);
    if (!q->buf) {
        free(q);
        return NULL;
    }
    return q;
}

void vQueueDelete(QueueHandle_t handle)
{
    struct host_queue *q = handle;
    if (!q) {
        return;
    }
    free(q->buf);
    free(q);
}

BaseType_t xQueueSend(QueueHandle_t handle, const void *item, TickType_t wait)
{
    (void)wait;
    struct host_queue *q = handle;
    if (!q || q->count >= q->len) {
        return pdFALSE;
    }
    memcpy(q->buf + q->count * q->item_size, item, q->item_size);
    q->count++;
    return pdTRUE;
}

BaseType_t xQueueReceive(QueueHandle_t handle, void *buf, TickType_t wait)
{
    (void)wait;
    struct host_queue *q = handle;
    if (!q || q->count == 0) {
        return pdFALSE;
    }
    memcpy(buf, q->buf, q->item_size);
    memmove(q->buf, q->buf + q->item_size, (q->count - 1) * q->item_size);
    q->count--;
    return pdTRUE;
}

SemaphoreHandle_t xSemaphoreCreateMutex(void)
{
    pthread_mutex_t *m = malloc(sizeof(*m));
    if (!m) {
        return NULL;
    }
    if (pthread_mutex_init(m, NULL) != 0) {
        free(m);
        return NULL;
    }
    return m;
}

BaseType_t xSemaphoreTake(SemaphoreHandle_t sem, TickType_t wait)
{
    (void)wait;
    if (!sem) {
        return pdFALSE;
    }
    return pthread_mutex_lock((pthread_mutex_t *)sem) == 0 ? pdTRUE : pdFALSE;
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t sem)
{
    if (!sem) {
        return pdFALSE;
    }
    return pthread_mutex_unlock((pthread_mutex_t *)sem) == 0 ? pdTRUE : pdFALSE;
}
