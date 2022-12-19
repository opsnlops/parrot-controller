
#pragma once

/**
* This file defines the particulars of this creature
*/


//#define USE_UART_CONTROL 1

#define NUMBER_OF_SERVOS    2
#define SERVO_HZ            50


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


class Creature {

};