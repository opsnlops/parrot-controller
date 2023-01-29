
#pragma once


/*
 * The TMC2209 API is in C, so let's make a wrapper-sort of construct to make it
 * accessible cleanly from C++.
 *
 * From the docs, at a min, these need to be implemented:
 *
 * // => UART wrapper
 * extern void tmc2209_readWriteArray(uint8_t channel, uint8_t *data, size_t writeLength, size_t readLength);
 * // <= UART wrapper
 *
 * // => CRC wrapper
 * extern uint8_t tmc2209_CRC8(uint8_t *data, size_t length);
 * // <= CRC wrapper
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

#include "tmc/ic/TMC2209/TMC2209_Constants.h"
#include "tmc/ic/TMC2209/TMC2209_Fields.h"
#include "tmc/ic/TMC2209/TMC2209_Register.h"
#include "tmc/ic/TMC2209/TMC2209.h"

void stepper_init(TMC2209TypeDef* tmc2209, uint8_t channel, uint8_t slave_address,
                  ConfigurationTypeDef* config, const int32_t *registerResetState);


#ifdef __cplusplus
}
#endif