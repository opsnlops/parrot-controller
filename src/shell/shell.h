
#pragma once

#include "controller-config.h"


#include "shell/shell.h"
#include "controller/controller.h"
#include "io/handler.h"
#include "creature/creature.h"


class DebugShell {

public:
    DebugShell(Creature* creature, Controller* controller, IOHandler* io);

    uint8_t init();
    uint8_t start();

    Creature* getCreature();
    Controller* getController();
    IOHandler* getIOHandler();


private:

    Creature* creature;
    Controller* controller;
    IOHandler* io;

};