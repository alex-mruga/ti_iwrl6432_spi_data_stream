#include "ti_drivers_config.h"

#include "uart_transmit.h"



void mmw_UartWrite(UART_Handle handle, uint8_t *payload, uint32_t payloadLength) {
    UART_Transaction trans;

    UART_Transaction_init(&trans);

    trans.buf   = payload;
    trans.count = payloadLength;

    UART_write(handle, &trans);
}
