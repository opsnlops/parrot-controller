

#include <cstdio>

#include "creature.h"

#include "logging/logging.h"
#include "dmx.h"
#include "tasks.h"


DmxInput dmx_input;
volatile uint8_t dmx_buffer[DMXINPUT_BUFFER_SIZE(DMX_BASE_CHANNEL, DMX_NUMBER_OF_CHANNELS)];
volatile uint32_t dmx_packets_read = 0;

extern TaskHandle_t dmx_processing_task_handle;

int dmx_init(int dmx_input_pin)
{
    info("Starting up DMS on pin %d", dmx_input_pin);

    int dmx_status = dmx_input.begin(dmx_input_pin, DMX_BASE_CHANNEL, DMX_NUMBER_OF_CHANNELS);

    if(dmx_status != dmx_input.SUCCESS) {
        error("Unable to start DMX! Error code: %d", dmx_status);
        return 0;
    }

    dmx_input.read_async(dmx_buffer, dmxDataGotten);

    debug("DMX started!");

    return 1;
}

BaseType_t xHigherPriorityTaskWoken = pdFALSE;
uint32_t ulStatusRegister = 0;
void __isr dmxDataGotten(DmxInput* instance) {

    dmx_packets_read++;
    xTaskNotifyFromISR(dmx_processing_task_handle,
                       ulStatusRegister,
                       eNoAction,
                       &xHigherPriorityTaskWoken);
}
