#ifndef UART_TRANSMIT_H
#define UART_TRANSMIT_H

#include "kernel/dpl/SemaphoreP.h"

/**
 * @brief Semaphore to signal the start of UART transmission.
 *
 * This semaphore is used to trigger the UART transmission process.
 */
extern SemaphoreP_Object uart_tx_start_sem;

/**
 * @brief Semaphore to signal the completion of UART transmission.
 *
 * This semaphore is posted when the UART transmission is complete.
 */
extern SemaphoreP_Object uart_tx_done_sem;

/**
 * @brief UART transmission loop function.
 *
 * This function continuously waits for a signal to start UART transmission,
 * sends radar cube data over UART, and signals completion when done.
 */
void uart_transmit_loop();

#endif /* UART_TRANSMIT_H */