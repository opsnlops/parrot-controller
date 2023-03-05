
#pragma once


#include "logging/logging.h"
#include "controller/controller.h"

class StatusLights {

public:
    StatusLights(Controller* controller, IOHandler* io);

    void init();
    void start();

    PIO pio;
    uint8_t state_machine;

    static inline void put_pixel(uint32_t pixel_grb);
    static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);

    uint32_t last_input_frame;


    Controller* getController();
    IOHandler* getIOHandler();

private:

    Controller* controller;
    IOHandler* io;

};