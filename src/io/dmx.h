

#pragma once

#include <cstdio>
#include "pico/stdlib.h"

#include "Pico-DMX/src/DmxInput.h"

#include "handler.h"



class DMX : public IOHandler {

public:

    DMX();
    int init() override;

    void setInputPin(int inputPin);

    static uint32_t messagesProcessed;
    uint32_t getNumberOfFramesReceived() override;

    static void __isr dmxDataGotten(DmxInput* instance);

private:
    int inputPin;

};



int dmx_init(int dmx_input_pin);
