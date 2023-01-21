
#pragma once

/**
* Main configuration for the controller
*/

// Just because it's funny
#define EVER ;;

//#define USE_UART_CONTROL 1

// The most servos we can control
#define MAX_NUMBER_OF_SERVOS        16

// Devices
#define DMX_PIO                     pio0
#define DMX_INVERTED                false
#define E_STOP_PIN                  28
#define DMX_GPIO_PIN                22

// Debugging shell (PIO-based UART)
#define DEBUG_SHELL_UART_PIO        pio1
#define DEBUG_SHELL_TX              26
#define DEBUG_SHELL_RX              27
#define DEBUG_SHELL_BAUD_RATE       57600

// Stepper (UART1)
#define STEPPER_UART                uart1
#define STEPPER_UART_BAUD_RATE      115200
#define STEPPER_PARITY              UART_PARITY_NONE
#define STEPPER_DATA_BITS           8
#define STEPPER_STOP_BITS           1
#define STEPPER_UART_TX             4
#define STEPPER_UART_RX             5


/**
 * These allow more than one creature to be controlled on the same
 * DMX universe!
 *
 * Everything is defined as an offset from the base. The values are
 * DMX_BASE_CHANNEL + the offset for the desired value.
 */
#define DMX_E_STOP_CHANNEL_OFFSET   0


/*
 * Display Stuff
 */

#define DISPLAY_I2C_BAUD_RATE       1000000
#define DISPLAY_I2C_CONTROLLER      i2c1
#define DISPLAY_I2C_DEVICE_ADDRESS  0x3C
#define DISPLAY_SDA_PIN             2
#define DISPLAY_SCL_PIN             3

// Update every 33ms (roughly 30Hz)
#define DISPLAY_UPDATE_TIME_MS      33
#define DISPLAY_BUFFER_SIZE         256     // This is per line
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
#define MAX_SERVO_POSITION          1023

/*
 * Servo <-> GPIO Pin Mappings
 */
#define SERVO_0_GPIO_PIN            6               // Pin 9,  PMW  3A
#define SERVO_1_GPIO_PIN            7               // Pin 10, PWM  3B
#define SERVO_2_GPIO_PIN            8               // Pin 11, PWM  4A
#define SERVO_3_GPIO_PIN            9               // Pin 12, PWM  4B
#define SERVO_4_GPIO_PIN            10              // Pin 14, PWM  5A
#define SERVO_5_GPIO_PIN            11              // Pin 15, PWM  5B
#define SERVO_6_GPIO_PIN            12              // Pin 16, PWM  6A
#define SERVO_7_GPIO_PIN            13              // Pin 17, PWM  6B
#define SERVO_8_GPIO_PIN            14              // Pin 19, PWM  7A
#define SERVO_9_GPIO_PIN            15              // Pin 20, PWM  7B

// This is the point where it wraps around to the other side of the PCB
#define SERVO_10_GPIO_PIN           16              // Pin 21, PWM  0A
#define SERVO_11_GPIO_PIN           17              // Pin 22, PWM  0B

// These last four can be SPI0 pins. If we need SPI, we have to limit
// ourselves to 12 servos.
#define SERVO_12_GPIO_PIN           18              // Pin 24, PWM  1A
#define SERVO_13_GPIO_PIN           19              // Pin 25, PWM  1B
#define SERVO_14_GPIO_PIN           20              // Pin 26, PWM  2A
#define SERVO_15_GPIO_PIN           21              // Pin 27, PWM  2B
