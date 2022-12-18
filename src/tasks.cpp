

#include "creature.h"

#include <FreeRTOS.h>
#include <task.h>



TaskHandle_t displayUpdateTaskHandle;
TaskHandle_t hellorldTaskHandle;
TaskHandle_t log_queue_reader_task_handle;

#ifdef USE_UART_CONTROL
TaskHandle_t messageQueueReaderTaskHandle;
#endif

TaskHandle_t servoDebugTaskHandle;
TaskHandle_t relayDebugTaskHandle;
