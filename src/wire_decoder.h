
#pragma once

#include "controller.h"


#include <climits>
#include <cstdio>
#include <unistd.h>

#ifdef USE_UART_CONTROL

uint16_t convert_dmx_position_to_servo_position(u_int8_t incoming_value);

void process_uart_frame(u_int8_t *buffer);

#endif