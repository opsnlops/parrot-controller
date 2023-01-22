

#include "controller-config.h"

#include "pico/stdlib.h"
#include "pico/time.h"

#include "tmc/ic/TMC2209/TMC2209.h"



#include "tmc2209_wrapper.h"


void stepper_init(TMC2209TypeDef* tmc2209, uint8_t channel, uint8_t slave_address,
                  ConfigurationTypeDef* config, const int32_t *registerResetState) {

    tmc2209_init(tmc2209, channel, slave_address, config, registerResetState);
    tmc2209_reset(tmc2209);
    tmc2209_periodicJob(tmc2209, to_ms_since_boot(get_absolute_time()));


}

/**
 * @brief Writes bytes to a UART, and then reads back a result
 *
 * @param channel channel to write to (if we had more than one, just use 0)
 * @param data buffer
 * @param writeLength how many bytes to write out of the buffer
 * @param readLength how many bytes to write INTO the buffer
 */
void tmc2209_readWriteArray(uint8_t channel, uint8_t *data, size_t writeLength, size_t readLength) {

    // Write the bytes
    for(int i = 0; i < writeLength; i++) {
        uart_putc_raw(STEPPER_UART, data[i]);
    }

    // Now go read the response
    for(int i = 0; i < readLength; i++) {
        data[i] = uart_getc(STEPPER_UART);
    }

}

// TODO: See if there's a better version of this somewhere
volatile uint8_t tmc2209_CRC8(const uint8_t *data, size_t len)
{
    size_t i;
    uint8_t j;
    uint8_t temp;
    uint8_t data_byte;
    uint8_t crc = 0;
    for (i = 0; i < len; i++)
    {
        data_byte = data[i];
        for (j = 0; j < 8; j++)
        {
            temp = (crc ^ data_byte) & 0x01;
            crc >>= 1;
            if (temp)
                crc ^= 0x8C;

            data_byte >>= 1;
        }
    }

    return crc;
}