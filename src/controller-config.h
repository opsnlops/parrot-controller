
#pragma once

/**
* Main configuration for the controller
*/

// Just because it's funny
#define EVER ;;

//#define USE_UART_CONTROL 1

// The most servos we can control
#define MAX_NUMBER_OF_SERVOS    16
#define SERVO_HZ                50


#define E_STOP_PIN          19
#define DMX_GPIO_PIN        18


/**
 * These allow more than one creature to be controlled on the same
 * DMX universe!
 *
 * Everything is defined as an offset from the base. The values are
 * DMX_BASE_CHANNEL + the offset for the desired value.
 */
#define DMX_BASE_CHANNEL            1
#define DMX_E_STOP_CHANNEL_OFFSET   0
#define DMX_SMOOTHING_OFFSET        1
#define DMX_HEAD_LEFT_OFFSET        2
#define DMX_HEAD_RIGHT_OFFSET       3


#define DMX_NUMBER_OF_CHANNELS      9





/*
 * Display Stuff
 */

// Update every 33ms (roughly 30Hz)
#define DISPLAY_UPDATE_TIME_MS     33
#define DISPLAY_BUFFER_SIZE        256


/*
 * Logging Config
 */
#define LOGGING_LEVEL               LOG_LEVEL_VERBOSE
#define LOGGING_QUEUE_LENGTH        10
#define LOGGING_MESSAGE_MAX_LENGTH  512

/*
 * Servo bounds
 */
#define MIN_SERVO_POSITION          0
#define MAX_SERVO_POSITION          999
