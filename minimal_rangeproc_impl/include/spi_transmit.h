#ifndef SPI_TRANSMIT_H
#define SPI_TRANSMIT_H

/**
 * @file spi_transmit.h
 * @brief SPI Transmission Interface for radar cube and raw ADC data.
 *
 * This file defines the interface for transmitting radar cube and ADC data over SPI.
 * It includes the declaration of semaphores used to synchronize the SPI
 * transmission process and the function prototype for the SPI transmission loop.
 *
 * The SPI transmission loop waits for a signal to start transmission, sends
 * radar cube data over SPI, and signals completion when done. This module
 * is designed to facilitate communication between the radar processing unit
 * and external systems via SPI.
 *
 * @note This module relies on the SemaphoreP API from the kernel/dpl library
 *       for synchronization.
*/

/**
 * @brief Semaphore to signal the start of SPI transmission.
 *
 * This semaphore is used to trigger the SPI transmission process.
 */
extern SemaphoreP_Object spi_tx_start_sem;

/**
 * @brief Semaphore to signal the completion of SPI transmission.
 *
 * This semaphore is posted when the SPI transmission is complete.
 */
extern SemaphoreP_Object spi_tx_done_sem;

/**
 * @brief SPI transmission loop function.
 *
 * This function continuously waits for a signal to start SPI transmission,
 * sends radar cube and ADC data over SPI, and signals completion when done.
 */
void spi_transmit_loop();

#endif /* SPI_TRANSMIT_H */