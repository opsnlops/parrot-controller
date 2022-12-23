
#pragma once

#include <cstdio>
#include <climits>
#include "pico/stdlib.h"

#include "Pico-DMX/src/DmxInput.h"

#include "handler.h"

#include "controller/controller.h"

// Used to pass information into the task
typedef struct {
    Controller* controller;
    uint8_t dmxOffset;
    volatile uint8_t*    dmx_buffer;
} DmxHandlerInfo;


class DMX : public IOHandler {

public:

    explicit DMX(Controller* controller);

    int init() override;
    int start() override;

    void setInputPin(int inputPin);

    static uint32_t messagesProcessed;
    uint32_t getNumberOfFramesReceived() override;

    static void __isr dmxDataGotten(DmxInput* instance);

private:
    int inputPin;

    DmxInput dmx_input;
    volatile uint8_t dmx_buffer[DMXINPUT_BUFFER_SIZE(DMX_BASE_CHANNEL, DMX_NUMBER_OF_CHANNELS)];

    Controller* controller;

};
