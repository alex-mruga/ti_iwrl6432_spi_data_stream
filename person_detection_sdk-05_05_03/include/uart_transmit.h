#ifndef UART_TRANSMIT_H
#define UART_TRANSMIT_H

#include "kernel/dpl/SemaphoreP.h"

extern SemaphoreP_Object uart_tx_start_sem;
extern SemaphoreP_Object uart_tx_done_sem;

void uart_transmit_loop();

#endif /* UART_TRANSMIT_H */