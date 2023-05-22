
#pragma once

#include <cstdio>
#include <climits>
#include <memory>
#include "pico/stdlib.h"

#include "Pico-DMX/src/DmxInput.h"

#include "handler.h"

#include "controller/controller.h"

// Used to pass information into the task
typedef struct {
    Controller* controller;
    uint16_t baseChannel;
    uint16_t numberOfChannels;
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

    uint16_t baseChannel;
    uint16_t numberOfChannels;

    DmxInput dmx_input;
    Controller* controller;

    PIO pio;

    volatile uint8_t dmx_buffer[];

};
