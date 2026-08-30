#pragma once

#include <stdint.h>

typedef int BaseType_t;
typedef unsigned long TickType_t;
typedef uint32_t UBaseType_t;

#define pdTRUE           1
#define pdFALSE          0
#define pdPASS           pdTRUE
#define pdFAIL           pdFALSE
#define portMAX_DELAY    ((TickType_t)0xffffffffUL)
#define portTICK_PERIOD_MS 1
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))
