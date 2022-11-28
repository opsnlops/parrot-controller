
#pragma once

#include <FreeRTOS.h>
#include <task.h>

// Just because it's funny
#define EVER ;;

// The header is defined in main.cpp
#define HEADER_SIZE 3

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
#define LOGGING_LEVEL               LOG_LEVEL_DEBUG
#define LOGGING_QUEUE_LENGTH        16
#define LOGGING_MESSAGE_MAX_LENGTH  512

/*
 * Servo bounds
 */
#define MIN_SERVO_POSITION          0
#define MAX_SERVO_POSITION          999

#define NUMBER_OF_SERVOS            2
#define SERVO_HZ                    50
