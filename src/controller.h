
#pragma once

#include <FreeRTOS.h>

// Just because it's funny
#define EVER ;;

/*
 * Display Stuff
 */

// Update every 33ms (roughly 30Hz)
#define DISPLAY_UPDATE_TIME_MS     33
#define DISPLAY_BUFFER_SIZE        256

#define INCOMING_CHARACTER_QUEUE_SIZE 64

/*
 * Logging Config
 */
#define LOGGING_LEVEL               5
#define LOGGING_QUEUE_LENGTH        16
#define LOGGING_MESSAGE_MAX_LENGTH  512

/*
 * Task Prototypes
 */
portTASK_FUNCTION_PROTO(displayUpdateTask, pvParameters);           // defined in display.cpp
portTASK_FUNCTION_PROTO(hellorldTask, pvParameters);                // defined in main.cpp
portTASK_FUNCTION_PROTO(logQueueReaderTask, pvParameters);          // defined in logging/logging.cpp
portTASK_FUNCTION_PROTO(messageQueueReaderTask, pvParameters);      // defined in logging/logging.cpp
