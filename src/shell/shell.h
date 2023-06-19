
#pragma once

#include "controller-config.h"


#include "shell/shell.h"
#include "controller/controller.h"
#include "io/handler.h"
#include "io/pio_uart.h"
#include "creature/config.h"
#include "creature/creature.h"

#define DS_TX_BUFFER_SIZE       1024
#define DS_RX_BUFFER_SIZE       16

class DebugShell {

public:
    DebugShell(Creature* creature, Controller* controller, IOHandler* io);

    static uint8_t init();
    uint8_t start();

    Creature* getCreature();
    Controller* getController();
    IOHandler* getIOHandler();
    CreatureConfig* getConfig();

    void showConfig(char* tx_buffer, uint8_t* rx_buffer);
    void showDebug(char* tx_buffer, uint8_t* rx_buffer);

private:

    Creature* creature;
    Controller* controller;
    IOHandler* io;
    CreatureConfig* config;

};


// Global scoped stuffs
void write_to_cdc(char* line);
void print_log_to_shell(char* logLine);
