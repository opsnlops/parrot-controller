

#include <FreeRTOS.h>
#include <task.h>



TaskHandle_t displayUpdateTaskHandle;
TaskHandle_t hellorldTaskHandle;
TaskHandle_t log_queue_reader_task_handle;
TaskHandle_t messageQueueReaderTaskHandle;
TaskHandle_t servoDebugTaskHandle;
