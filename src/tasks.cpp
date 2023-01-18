

#include "controller-config.h"

#include <FreeRTOS.h>
#include <task.h>


TaskHandle_t debug_console_task_handle;
TaskHandle_t displayUpdateTaskHandle;
TaskHandle_t log_queue_reader_task_handle;
TaskHandle_t dmx_processing_task_handle;
TaskHandle_t stepper_uart_task_handle;
volatile TaskHandle_t controllerHousekeeperTaskHandle;

#ifdef USE_UART_CONTROL
TaskHandle_t messageQueueReaderTaskHandle;
#endif
