
#include "src/creature.h"
#include "src/controller.h"

#include "uart.h"

#include <fcntl.h>
#include <cassert>
#include <climits>
#include <cstdio>
#include <unistd.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "src/logging/logging.h"
#include "src/servo.h"


#ifdef USE_UART_CONTROL

// All of these are in main.cpp
extern QueueHandle_t incomingQueue;
extern Servo servos[NUMBER_OF_SERVOS];

// HOP
u_int8_t header[HEADER_SIZE] = {0x48, 0x4F, 0x50};



// Read from the queue and print it to the screen for now
portTASK_FUNCTION(messageQueueReaderTask, pvParameters) {

    uint8_t incoming;

    // Keep a buffer of what we've gotten
    uint8_t buffer[HEADER_SIZE + NUMBER_OF_SERVOS + 1];
    memset(&buffer, '\0', HEADER_SIZE + NUMBER_OF_SERVOS + 1);


    uint8_t current_position = 0;

    bool discard_until_null = false;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {
        if (xQueueReceive(incomingQueue, &incoming, (TickType_t) portMAX_DELAY) == pdPASS) {
            //printf("incoming: 0x%x, current_position: %d\n", incoming, current_position);

            if (discard_until_null) {
                // is this a null?
                if (incoming != 0x00)
                    continue;
                else
                    discard_until_null = false;
            } else {

                // If we have a null, and we're not at the end of the frame, something went wrong
                if (current_position == HEADER_SIZE + NUMBER_OF_SERVOS && incoming != 0x00) {

                    warning("at the end of the frame and got something other than 0x00. something's wrong. discarding until we find a 0x00.");
                    current_position = 0;
                    memset(&buffer, '\0', HEADER_SIZE + NUMBER_OF_SERVOS + 1);

                    discard_until_null = true;

                } else {

                    // Do we still have room in the buffer?
                    if (current_position <= HEADER_SIZE + NUMBER_OF_SERVOS) {

                        // Toss this in the buffer
                        buffer[current_position++] = incoming;
                    }

                    if (current_position == HEADER_SIZE + NUMBER_OF_SERVOS + 1) {

                        debug("looks like we got a full frame, processing!");
                        process_uart_frame(buffer);

                        // Reset the buffer
                        current_position = 0;
                        memset(&buffer, '\0', HEADER_SIZE + NUMBER_OF_SERVOS + 1);
                    }
                }
            }
        }
    }
#pragma clang diagnostic pop

}


uint16_t convert_dmx_position_to_servo_position(u_int8_t incoming_value) {

    float percentage_requested = (float) incoming_value / (float) UCHAR_MAX;

    uint16_t local_requested = MAX_SERVO_POSITION * percentage_requested;

    verbose("converted %d -> %d", incoming_value, local_requested);

    return local_requested;

}

void process_uart_frame(u_int8_t *buffer) {

    debug("processing frame!");

    // We've already verified the last byte, so let's check the header
    if(memcmp(reinterpret_cast<char*>(buffer), reinterpret_cast<char*>(header), HEADER_SIZE) == 0)
    {
        debug("valid frame");

        for(int i = HEADER_SIZE; i < HEADER_SIZE + NUMBER_OF_SERVOS; i++)
        {
            servo_move(&servos[i - HEADER_SIZE], convert_dmx_position_to_servo_position(buffer[i]));
        }

    }
    else
    {
        warning("header not found in frame; dropping");
    }



}

#endif