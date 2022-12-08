

#pragma once

#include <FreeRTOS.h>
#include <task.h>


/*
 * This file is a crude sort of task manager, listing all the tasks
 * we have.
 */


portTASK_FUNCTION_PROTO(displayUpdateTask, pvParameters);           // used in display.cpp
portTASK_FUNCTION_PROTO(hellorldTask, pvParameters);                // used in main.cpp
portTASK_FUNCTION_PROTO(log_queue_reader_task, pvParameters);       // used in logging/logging.cpp
portTASK_FUNCTION_PROTO(messageQueueReaderTask, pvParameters);      // used in logging/logging.cpp

// Testing only
portTASK_FUNCTION_PROTO(servoDebugTask, pvParameters);              // used in main.cpp
