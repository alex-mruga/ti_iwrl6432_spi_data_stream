/**
 * @file uart_transmit.c
 * @brief UART Transmission Implementation for Radar Data.
 *
 * This file implements the UART transmission of radar cube data.
 * It manages synchronization using semaphores, waits for transmission signals,
 * sends data over UART, and signals completion when done.
 *
 * The transmission process is controlled using two semaphores:
 * - `uart_tx_start_sem`: Signals the start of transmission.
 * - `uart_tx_done_sem`: Signals the completion of transmission.
 *
 * The function `uart_transmit_loop()` runs continuously, waiting for
 * `uart_tx_start_sem` to be posted, transmitting radar cube data, and posting
 * `uart_tx_done_sem` upon completion.
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

#include "defines.h"
#include "uart_transmit.h"



#define APP_UART_BUFSIZE              (1024)
#define APP_UART_RECEIVE_BUFSIZE      (8U)

uint8_t gUartBuffer[APP_UART_BUFSIZE];
uint8_t gUartReceiveBuffer[APP_UART_RECEIVE_BUFSIZE];
volatile uint32_t gNumBytesRead = 0U, gNumBytesWritten = 0U;

/**
 * @brief Header and footer for each radarCube transmission over UART.
 *
 * These markers are used to indicate the start and end of each radarCube 
 * data packet during UART transmission.
 */
const uint8_t header[4] = {0xAA, 0xBB, 0xCC, 0xDD};
const uint8_t footer[4] = {0xDD, 0xCC, 0xBB, 0xAA};

extern DPU_RangeProcHWA_Config rangeProcDpuCfg;


void uart_transmit_loop(){
    cmplx16ImRe_t *radarCube = rangeProcDpuCfg.hwRes.radarCube.data;
    
    int32_t          transferOK;
    UART_Transaction trans;

    UART_Transaction_init(&trans);


    while(true){
        SemaphoreP_pend(&uart_tx_start_sem, SystemP_WAIT_FOREVER);

        gNumBytesWritten = 0U;
        trans.buf   = &gUartBuffer[0U];

        // send header
        trans.buf = (void *) &header;
        trans.count = 4;
        transferOK = UART_write(gUartHandle[CONFIG_UART_CONSOLE], &trans);
        if (transferOK != SystemP_SUCCESS){
            DebugP_log("Uart Tx failed");
        }

        // send actual radar cube data
        // only send data of one virtual antenna though, because only range fft is transmitted for now.
        uint32_t i;
        for (i = 0; i < NUM_RANGE_BINS; i++) {
            
            // data structure in radarCube:
            // Cube[chirp][antenna][range]

            // calculate address offset of rangebin
            uint8_t chirp_index = 1;
            uint8_t antenna_index = 0;
            uint8_t rangebin_index = i;

            uint32_t address_offset = (chirp_index * antenna_index * rangebin_index) + (antenna_index * rangebin_index) + rangebin_index;

            trans.buf = (void *) &radarCube[address_offset];
            trans.count = 4;
            transferOK = UART_write(gUartHandle[CONFIG_UART_CONSOLE], &trans);
            if (transferOK != SystemP_SUCCESS){
            DebugP_log("Uart Tx failed");
        }
        }

        // send footer
        trans.buf = (void *) &footer;
        trans.count = 4;
        transferOK = UART_write(gUartHandle[CONFIG_UART_CONSOLE], &trans);
        if (transferOK != SystemP_SUCCESS){
            DebugP_log("Uart Tx failed");
        }

        SemaphoreP_post(&uart_tx_done_sem);
    }
}