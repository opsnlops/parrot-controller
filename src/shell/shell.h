
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
    DebugShell(std::shared_ptr<Creature> creature, std::shared_ptr<Controller> controller, IOHandler* io);

    static uint8_t init();
    uint8_t start();

    std::shared_ptr<Creature> getCreature();
    std::shared_ptr<Controller> getController();
    IOHandler* getIOHandler();
    std::shared_ptr<CreatureConfig> getConfig();

private:

    std::shared_ptr<Creature> creature;
    std::shared_ptr<Controller> controller;
    IOHandler* io;
    std::shared_ptr<CreatureConfig> config;

};


// Global scoped stuffs
void write_to_cdc(char* line);
void print_log_to_shell(char* logLine);
