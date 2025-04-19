/**
 * @file spi_transmit.c
 * @brief SPI Transmission Implementation for Radar Data.
 *
 * This file implements the SPI transmission of radar cube and raw ADC data.
 * It manages synchronization using semaphores, waits for transmission signals,
 * sends data over SPI, and signals completion when done.
 *
 * The transmission process is controlled using two semaphores:
 * - `spi_tx_start_sem`: Signals the start of transmission.
 * - `spi_tx_done_sem`: Signals the completion of transmission.
 *
 * The function `spi_transmit_loop()` runs continuously, waiting for
 * `spi_tx_start_sem` to be posted, transmitting radar cube data, and posting
 * `spi_tx_done_sem` upon completion.
 *
 * @note This module relies on the SemaphoreP API from the kernel/dpl library
 *       for synchronization.
 */

#include "ti_drivers_config.h"
#include <stdio.h>
#include "string.h"
#include <kernel/dpl/DebugP.h>
#include <utils/mathutils/mathutils.h>
#include "kernel/dpl/SemaphoreP.h"
#include "ti_drivers_open_close.h"
#include <datapath/dpu/rangeproc/v0/rangeprochwa.h>

#include "system.h"
#include "defines.h"
#include "spi_transmit.h"


// #define APP_UART_BUFSIZE              (1024)
// #define APP_UART_RECEIVE_BUFSIZE      (8U)

/**
 * @brief Header and footer for each radarCube transmission over SPI.
 *
 * These markers are used to indicate the start and end of each frame 
 * data packet during SPI transmission.
 */
const uint8_t header[4] = {0xAA, 0xBB, 0xCC, 0xDD};
const uint8_t footer[4] = {0xDD, 0xCC, 0xBB, 0xAA};


void spi_transmit_loop() {
    cmplx16ImRe_t *radarCube = gSysContext.rangeProcDpuCfg.hwRes.radarCube.data;

    while(true) {
        SemaphoreP_pend(&spi_tx_start_sem, SystemP_WAIT_FOREVER);

        // TODO

        SemaphoreP_post(&spi_tx_done_sem);
    }
}