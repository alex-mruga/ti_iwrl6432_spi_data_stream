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


#define MAX_SPI_TRANSFER_SIZE       (65536U) // Max. Bytes per transfer burst (FTDI: 65536 Byte)
#define BITS_PER_FRAME              (32U)    // Bits per SPI frame */
#define BYTES_PER_FRAME             (BITS_PER_FRAME/8U)

static int32_t spi_transfer_buffer(void *txBuf, uint32_t totalBytes) {
    MCSPI_Transaction spiTransaction;
    int32_t           transferOK;
    uint32_t          bytesRemaining = totalBytes;
    uint32_t          chunkIndex     = 0;

    while (bytesRemaining > 0) {
        // determine this chunk's byte size
        uint32_t chunkSize;
        if (bytesRemaining > MAX_SPI_TRANSFER_SIZE) {
            // data is larger than one transfer can handle
            chunkSize = MAX_SPI_TRANSFER_SIZE;
        } else {
            // data fits in a single transfer
            chunkSize = bytesRemaining;
        }

        // number of 32-bit frames in this chunk
        uint32_t frameCount = chunkSize / BYTES_PER_FRAME;

        // calculate byte offset into buffer
        uint32_t byteOffset = MAX_SPI_TRANSFER_SIZE * chunkIndex;

        // initiate SPI transfer
        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel   = gConfigMcspi0ChCfg[0].chNum;
        spiTransaction.dataSize  = BITS_PER_FRAME;
        spiTransaction.csDisable = TRUE;  // CS low during transfer
        spiTransaction.count     = frameCount;
        spiTransaction.txBuf     = (void *)((uint8_t *)txBuf + byteOffset);
        spiTransaction.rxBuf     = NULL;
        spiTransaction.args      = NULL;

        transferOK = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        if (transferOK != SystemP_SUCCESS) {
            return transferOK;
        }

        // update for next chunk
        bytesRemaining -= chunkSize;
        chunkIndex++;
    }

    return SystemP_SUCCESS;
}

void spi_transmit_loop() {
    int32_t           transferOK;

    // assign pointers which point to data which is transferred
    cmplx16ImRe_t *radarCubePtr = gSysContext.rangeProcDpuCfg.hwRes.radarCube.data;
    //uint8_t *adcData           = 

    // Total bytes in one radar-cube frame
    uint32_t radarCubeBytes = gSysContext.rangeProcDpuCfg.hwRes.radarCube.dataSize \
                               * sizeof(cmplx16ImRe_t);


    while(true) {
        // wait for new frame to be captured
        SemaphoreP_pend(&spi_tx_start_sem, SystemP_WAIT_FOREVER);

        // transfer the radar cube via SPI
        transferOK = spi_transfer_buffer(radarCubePtr, radarCubeBytes);
        if (transferOK != SystemP_SUCCESS) {
            DebugP_log("SPI radar cube data transfer failed\r\n");
        }

        // TODO: transfer raw ADC data via SPI

        SemaphoreP_post(&spi_tx_done_sem);
    }
}