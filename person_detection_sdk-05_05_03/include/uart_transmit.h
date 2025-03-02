#ifndef UART_TRANSMIT_H
#define UART_TRANSMIT_H

/**
 * @file uart_transmit.h
 * @brief UART Transmission Interface for Radar Data.
 *
 * This file defines the interface for transmitting radar cube data over UART.
 * It includes the declaration of semaphores used to synchronize the UART
 * transmission process and the function prototype for the UART transmission loop.
 *
 * The UART transmission loop waits for a signal to start transmission, sends
 * radar cube data over UART, and signals completion when done. This module
 * is designed to facilitate communication between the radar processing unit
 * and external systems via UART.
 *
 * @note This module relies on the SemaphoreP API from the kernel/dpl library
 *       for synchronization.
*/

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