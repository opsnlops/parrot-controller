
#pragma once

#include "controller-config.h"


#include "shell/shell.h"
#include "controller/controller.h"
#include "io/handler.h"
#include "creature/config.h"
#include "creature/creature.h"

#define DS_TX_BUFFER_SIZE       1024
#define DS_RX_BUFFER_SIZE       16

class DebugShell {

public:
    DebugShell(Creature* creature, Controller* controller, IOHandler* io);

    uint8_t init();
    uint8_t start();

    Creature* getCreature();
    Controller* getController();
    IOHandler* getIOHandler();
    CreatureConfig* getConfig();


private:

    Creature* creature;
    Controller* controller;
    IOHandler* io;
    CreatureConfig* config;

};