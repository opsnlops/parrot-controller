

#include <cstdio>

#include "logging/logging.h"
#include "dmx.h"


#include <DmxInput.h>


DmxInput dmx_input;
volatile uint8_t dmx_buffer[DMXINPUT_BUFFER_SIZE(DMX_START_CHANNEL, DMX_NUM_CHANNELS)];

volatile uint32_t dmx_packets_read = 0;

int dmx_init(int dmx_input_pin)
{
    info("Starting up DMS on pin %d", dmx_input_pin);

    int dmx_status = dmx_input.begin(dmx_input_pin, DMX_START_CHANNEL, DMX_NUM_CHANNELS);

    if(dmx_status != dmx_input.SUCCESS) {
        error("Unable to start DMX! Error code: %d", dmx_status);
        return 0;
    }
    debug("DMX started!");

    return 1;
}


portTASK_FUNCTION(dmx_reader_task, pvParameters) {

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
   for(EVER)
    {
       debug("dmx read!");
       dmx_input.read(dmx_buffer);
       dmx_packets_read++;
    }
#pragma clang diagnostic pop
}