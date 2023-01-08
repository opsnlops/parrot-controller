
#include <cstdio>
#include <climits>

#include "controller-config.h"

#include "logging/logging.h"
#include "dmx.h"
#include "tasks.h"


// Statics
uint32_t DMX::messagesProcessed;

extern TaskHandle_t dmx_processing_task_handle;

DMX::DMX(Controller* controller) {
    inputPin = 0;
    messagesProcessed = 0;
    this->controller = controller;
    debug("new DMX just dropped");
}

void DMX::setInputPin(int input_pin) {
    this->inputPin = input_pin;
}

uint32_t DMX::getNumberOfFramesReceived() {
    return messagesProcessed;
}

int DMX::init()
{
    // TODO: Assert that the input pin is assigned


    info("Starting up DMX on pin %d", inputPin);

    this->baseChannel = controller->getConfig()->getDmxBaseChannel();
    this->numberOfChannels = controller->getConfig()->getNumberOfServos() + 1; // The number of servos + the e-stop

    dmx_buffer[DMXINPUT_BUFFER_SIZE(this->baseChannel,
                                    this->numberOfChannels)];

    return 1;
}

int DMX::start() {

    // Allocate the info on the heap, so it'll still be there when this function goes out
    // of scope. (As it would as a stack var.)
    auto *info = (DmxHandlerInfo*)pvPortMalloc(sizeof(DmxHandlerInfo));
    info->controller = this->controller;
    info->dmx_buffer = this->dmx_buffer;
    info->baseChannel = this->baseChannel;
    info->numberOfChannels = this->numberOfChannels;

    int dmx_status = dmx_input.begin(inputPin,
                                     this->baseChannel,
                                     this->numberOfChannels);

    if(dmx_status != dmx_input.SUCCESS) {
        error("Unable to start DMX! Error code: %d", dmx_status);
        return 0;
    }

    dmx_input.read_async(dmx_buffer, dmxDataGotten);
    debug("DMX started!");

    xTaskCreate(dmx_processing_task,
                "dmx_processing_task",
                2048,
                (void*)(info),
                1,
                &dmx_processing_task_handle);

    return 1;
}



/**
 * An ISR that's called when we receive data from DMX
 *
 * @param instance a reference to the `DmxInput` object
 */
void __isr DMX::dmxDataGotten(DmxInput* instance) {

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t ulStatusRegister = 0;

    // Send the processor a message
    xTaskNotifyFromISR(dmx_processing_task_handle,
                       ulStatusRegister,
                       eNoAction,
                       &xHigherPriorityTaskWoken);
    DMX::messagesProcessed++;
}


/**
 * This task blocks most of the time, until it's signalled by the DMX input ISR. Once it
 * gets the signal it copies the data out of the DMX library into one of our own buffers,
 * and then feeds it to the controller.
 *
 * @param pvParameters A pointer to a `DmxHandlerInfo` struct
 */
portTASK_FUNCTION(dmx_processing_task, pvParameters) {

    auto* info = (DmxHandlerInfo*)pvParameters;
    Controller* controller = info->controller;
    uint16_t baseChannel = info->baseChannel;
    uint16_t numberOfChannels = info->numberOfChannels;
    volatile uint8_t* dmx_buffer = info->dmx_buffer;

    // We're done with info, free it!
    vPortFree(info);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    // Create a buffer to copy the dmx info into
    auto buffer = (uint8_t*)pvPortMalloc(sizeof(uint8_t) * numberOfChannels);

    uint32_t ulNotifiedValue;
    for (EVER) {

        // Initialize the buffer
        for(int i = 0; i < numberOfChannels; i++) {
            buffer[i] = (uint8_t)0;
        }

        // Now wait to be signaled
        xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);

        // Copy the buffer info our local buffer
        for(int i = 0; i < numberOfChannels; i++) {
            buffer[i] = dmx_buffer[i + baseChannel];
        }

        // Send this to the controller
        controller->acceptInput(buffer);

    }
#pragma clang diagnostic pop
}