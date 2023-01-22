
#pragma once

#include "tmc/ic/TMC2209/TMC2209_Constants.h"
#include "tmc/ic/TMC2209/TMC2209_Fields.h"
#include "tmc/ic/TMC2209/TMC2209_Register.h"
#include "tmc/ic/TMC2209/TMC2209.h"


#include <cstdio>
#include "pico/stdlib.h"


class Stepper {

public:
    int init();
    int start();

    TMC2209TypeDef stepper1;
    ConfigurationTypeDef stepper_config;

};

