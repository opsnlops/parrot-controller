
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

#define DMX_NUMBER_OF_CHANNELS      9





/*
 * Display Stuff
 */

// Update every 33ms (roughly 30Hz)
#define DISPLAY_UPDATE_TIME_MS      33
#define DISPLAY_BUFFER_SIZE         256
#define DISPLAY_NUMBER_OF_LINES     4


/*
 * Logging Config
 */
#define LOGGING_LEVEL               LOG_LEVEL_DEBUG
#define LOGGING_QUEUE_LENGTH        40
#define LOGGING_MESSAGE_MAX_LENGTH  256

/*
 * Servo bounds
 */
#define MIN_SERVO_POSITION          0
#define MAX_SERVO_POSITION          999

/*
 * Servo <-> GPIO Pin Mappings
 */
#define SERVO_0_GPIO_PIN            22
#define SERVO_1_GPIO_PIN            1
#define SERVO_2_GPIO_PIN            2
#define SERVO_3_GPIO_PIN            3
#define SERVO_4_GPIO_PIN            4
#define SERVO_5_GPIO_PIN            5
#define SERVO_6_GPIO_PIN            6
#define SERVO_7_GPIO_PIN            7
#define SERVO_8_GPIO_PIN            8
#define SERVO_9_GPIO_PIN            9
#define SERVO_10_GPIO_PIN           10
#define SERVO_11_GPIO_PIN           11
#define SERVO_12_GPIO_PIN           12
#define SERVO_13_GPIO_PIN           13
#define SERVO_14_GPIO_PIN           14
#define SERVO_15_GPIO_PIN           15
