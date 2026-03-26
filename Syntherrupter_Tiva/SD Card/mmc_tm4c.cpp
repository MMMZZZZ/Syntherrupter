/**
 * Created using Claude Sonnet 4.6
 * 
 * mmc_tm4c.cpp - FatFs Disk I/O Driver for TI TM4C1294
 *                SD-Card over SPI
 *
 * Integration within FatFs:
 *   This file implements the functions from diskio.h:
 *     disk_initialize(), disk_status(), disk_read(),
 *     disk_write(), disk_ioctl(), get_fattime()
 *
 * Based on the FatFs Generic MMC/SPI Driver from ChaN,
 * adapted for TivaWare on the TM4C1294.
 */

#include "diskio.h"
#include "ff.h"

#include <stdint.h>
#include <stdbool.h>

// TivaWare DriverLib
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

// Syntherrupter integration
#include "System.h"

// =======================================================================
// Configuration
// =======================================================================

#define SD_SSI_BASE         SSI3_BASE
#define SD_SSI_SYSCTL       SYSCTL_PERIPH_SSI3
#define SD_GPIO_SYSCTL      SYSCTL_PERIPH_GPIOQ

#define SD_GPIO_PORT        GPIO_PORTQ_BASE

// Pins
#define SD_PIN_CLK          GPIO_PIN_0   
#define SD_PIN_CS           GPIO_PIN_1   
#define SD_PIN_MISO         GPIO_PIN_3   
#define SD_PIN_MOSI         GPIO_PIN_2   

// SPI Clock: Initialize at 400 kHz, the switch to 12 MHz
#define SD_SPI_BITRATE_INIT    400000UL
#define SD_SPI_BITRATE_FAST  12000000UL


// =======================================================================
// SD Card Commands
// =======================================================================
#define CMD0    (0)         // GO_IDLE_STATE
#define CMD1    (1)         // SEND_OP_COND (MMC)
#define ACMD41  (0x80 + 41) // SEND_OP_COND (SDC)
#define CMD8    (8)         // SEND_IF_COND
#define CMD9    (9)         // SEND_CSD
#define CMD10   (10)        // SEND_CID
#define CMD12   (12)        // STOP_TRANSMISSION
#define CMD13   (13)        // SEND_STATUS
#define ACMD13  (0x80 + 13) // SD_STATUS
#define CMD16   (16)        // SET_BLOCKLEN
#define CMD17   (17)        // READ_SINGLE_BLOCK
#define CMD18   (18)        // READ_MULTIPLE_BLOCK
#define CMD23   (23)        // SET_BLOCK_COUNT (MMC)
#define ACMD23  (0x80 + 23) // SET_WR_BLK_ERASE_COUNT (SDC)
#define CMD24   (24)        // WRITE_BLOCK
#define CMD25   (25)        // WRITE_MULTIPLE_BLOCK
#define CMD32   (32)        // ERASE_ER_BLK_START
#define CMD33   (33)        // ERASE_ER_BLK_END
#define CMD38   (38)        // ERASE
#define CMD55   (55)        // APP_CMD
#define CMD58   (58)        // READ_OCR

// Card type flags
#define CT_MMC      0x01
#define CT_SD1      0x02
#define CT_SD2      0x04
#define CT_SDC      (CT_SD1 | CT_SD2)
#define CT_BLOCK    0x08

// =======================================================================
// Module variables
// =======================================================================
static volatile DSTATUS disk_stat = STA_NOINIT;
static uint8_t          card_type = 0;


// =======================================================================
// Low level SPI / GPIO
// =======================================================================

static void spi_init(uint32_t bitrate)
{
    // Enable peripherals
    SysCtlPeripheralEnable(SD_SSI_SYSCTL);
    SysCtlPeripheralEnable(SD_GPIO_SYSCTL);

    // Configure CS pin as GPIO
    GPIOPinTypeGPIOOutput(SD_GPIO_PORT, SD_PIN_CS);
    GPIOPinWrite(SD_GPIO_PORT, SD_PIN_CS, SD_PIN_CS); // CS high

    // SSI pin configruation
    GPIOPinConfigure(GPIO_PQ0_SSI3CLK);
    GPIOPinConfigure(GPIO_PQ2_SSI3XDAT0);
    GPIOPinConfigure(GPIO_PQ3_SSI3XDAT1);
    GPIOPinTypeSSI(SD_GPIO_PORT, SD_PIN_CLK | SD_PIN_MISO | SD_PIN_MOSI);

    // SSI configuration: Motorola SPI, Mode 0, 8-Bit
    SSIDisable(SD_SSI_BASE);
    SSIConfigSetExpClk(SD_SSI_BASE, System::getClockFreq(), SSI_FRF_MOTO_MODE_0,
                           SSI_MODE_MASTER, bitrate, 8);
    SSIEnable(SD_SSI_BASE);

    // Flush RX FIFO 
    uint32_t tmp;
    while (MAP_SSIDataGetNonBlocking(SD_SSI_BASE, &tmp));
}

static inline void cs_low(void)
{
    GPIOPinWrite(SD_GPIO_PORT, SD_PIN_CS, 0);
}

static inline void cs_high(void)
{
    GPIOPinWrite(SD_GPIO_PORT, SD_PIN_CS, SD_PIN_CS);
}

static uint8_t spi_xchg(uint8_t data)
{
    uint32_t rx;
    SSIDataPut(SD_SSI_BASE, data);
    SSIDataGet(SD_SSI_BASE, &rx);
    return (uint8_t)rx;
}

static void spi_rcvr_multi(uint8_t* buf, uint32_t cnt)
{
    while (cnt--)
    {
        *buf++ = spi_xchg(0xFF);
    }
}

static void spi_xmit_multi(const uint8_t* buf, uint32_t cnt)
{
    while (cnt--)
    {
        spi_xchg(*buf++);
    }
}

// =======================================================================
// SD Protocol Helper Functions
// =======================================================================

/** Wait for card to become ready (non busy), Timeout in ms */
static bool wait_ready(uint32_t ms)
{
    uint32_t start = System::getSystemTimeMS();
    uint8_t d = 0;
    while (d != 0xFF && (System::getSystemTimeMS() - start) < ms)
    {
        d = spi_xchg(0xFF);
    }
    return (d == 0xFF);
}

static void deselect(void)
{
    cs_high();
    spi_xchg(0xFF); // One extra byte for SCK-Takt
}

/** Set CS and wait for ready */
static bool select(void)
{
    cs_low();
    spi_xchg(0xFF);
    if (wait_ready(500))
    {
        return true;
    }
    deselect();
    return false;
}

/** Read one data block from the card */
static bool rcvr_datablock(uint8_t* buf, uint32_t btr)
{
    // Wait for data token 0xFE
    uint32_t start = System::getSystemTimeMS();
    uint8_t token = 0xFF;
    while (token == 0xFF && (System::getSystemTimeMS() - start) < 200)
    {
        token = spi_xchg(0xFF);
    }

    if (token != 0xFE)
    {
        return false;
    }

    spi_rcvr_multi(buf, btr);
    spi_xchg(0xFF); // CRC (2 bytes, ignored)
    spi_xchg(0xFF);
    return true;
}

/** Write one data block to the card */
static bool xmit_datablock(const uint8_t* buf, uint8_t token)
{
    if (!wait_ready(500))
    {
        return false;
    }

    spi_xchg(token); // Send data token

    if (token == 0xFD)
    {
        return true; // Stop token – no data block
    }

    spi_xmit_multi(buf, 512);
    spi_xchg(0xFF); // Dummy CRC
    spi_xchg(0xFF);

    uint8_t resp = spi_xchg(0xFF) & 0x1F;
    return (resp == 0x05); // 0x05 = Data accepted
}

/** Send one SD command and return R1 response */
static uint8_t send_cmd(uint8_t cmd, uint32_t arg)
{
    // ACMD -> send CMD55 first
    if (cmd & 0x80)
    {
        cmd &= 0x7F;
        uint8_t res = send_cmd(CMD55, 0);
        if (res > 1)
        {
            return res;
        }
    }

    // De- and reselect CS
    deselect();
    if (!select())
    {
        return 0xFF;
    }

    // Send command frame
    spi_xchg(0x40 | cmd);
    spi_xchg((uint8_t)(arg >> 24));
    spi_xchg((uint8_t)(arg >> 16));
    spi_xchg((uint8_t)(arg >>  8));
    spi_xchg((uint8_t)(arg      ));

    // CRC (only relevant for CMD0 and CMD8, otherwise dummy 0x01)
    uint8_t crc = 0x01;
    if (cmd == CMD0)
    {
        crc = 0x95;
    }
    else if (cmd == CMD8)
    {
        crc = 0x87;
    }
    spi_xchg(crc);

    // CMD12: extra dummy byte
    if (cmd == CMD12)
    {
        spi_xchg(0xFF);
    }

    // Wait for R1 (max. 10 tries)
    uint8_t res;
    uint8_t n = 10;
    do
    {
        res = spi_xchg(0xFF);
    } while ((res & 0x80) && --n);

    return res;
}

// =======================================================================
// FatFs diskio Interface
// =======================================================================

DSTATUS disk_initialize(BYTE pdrv)
{
    if (pdrv != 0) return STA_NOINIT;

    // Initialize SPI with lowest frequency
    spi_init(SD_SPI_BITRATE_INIT);

    // Put card in SPI mode: >74 dummy clocks without CS
    cs_high();
    for (uint8_t i = 0; i < 10; i++) spi_xchg(0xFF);

    uint8_t  n, cmd, ty = 0;
    uint8_t  ocr[4];
    uint32_t start;

    // GO_IDLE_STATE -> card is in SPI mode
    if (send_cmd(CMD0, 0) == 1) {

        start = System::getSystemTimeMS();

        if (send_cmd(CMD8, 0x1AA) == 1) {
            // SD v2 – read interface condition
            for (n = 0; n < 4; n++) ocr[n] = spi_xchg(0xFF);

            if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
                // Send ACMD41 until card is ready (1s timeout)
                while ((System::getSystemTimeMS() - start) < 1000 &&
                       send_cmd(ACMD41, 0x40000000));

                if ((System::getSystemTimeMS() - start) < 1000 &&
                    send_cmd(CMD58, 0) == 0) {
                    // Read OCR -> detect SDHC/SDXC
                    for (n = 0; n < 4; n++) ocr[n] = spi_xchg(0xFF);
                    ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
                }
            }
        } else {
            // SD v1 or MMC
            if (send_cmd(ACMD41, 0) <= 1) {
                ty  = CT_SD1;
                cmd = ACMD41; // SD v1
            } else {
                ty  = CT_MMC;
                cmd = CMD1;   // MMC
            }
            while ((System::getSystemTimeMS() - start) < 1000 && send_cmd(cmd, 0));

            if ((System::getSystemTimeMS() - start) >= 1000 ||
                send_cmd(CMD16, 512) != 0) {
                ty = 0; // Initialization failed
            }
        }
    }

    card_type = ty;
    deselect();

    if (ty) {
        // Switch to high SPI frequency
        SSIDisable(SD_SSI_BASE);
        SSIConfigSetExpClk(SD_SSI_BASE, System::getClockFreq(), SSI_FRF_MOTO_MODE_0,
                               SSI_MODE_MASTER, SD_SPI_BITRATE_FAST, 8);
        SSIEnable(SD_SSI_BASE);

        disk_stat &= ~STA_NOINIT;
    } else {
        disk_stat = STA_NOINIT;
    }

    return disk_stat;
}

DSTATUS disk_status(BYTE pdrv)
{
    if (pdrv != 0) return STA_NOINIT;
    return disk_stat;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
    if (pdrv != 0 || (disk_stat & STA_NOINIT)) return RES_NOTRDY;
    if (!count) return RES_PARERR;

    // Byte addressing for non-block-based cards
    if (!(card_type & CT_BLOCK)) sector *= 512;

    DRESULT res = RES_ERROR;

    if (count == 1) {
        if (send_cmd(CMD17, sector) == 0 && rcvr_datablock(buff, 512)) {
            res = RES_OK;
        }
    } else {
        if (send_cmd(CMD18, sector) == 0) {
            do {
                if (!rcvr_datablock(buff, 512)) break;
                buff += 512;
            } while (--count);
            send_cmd(CMD12, 0); // STOP_TRANSMISSION
            if (!count) res = RES_OK;
        }
    }

    deselect();
    return res;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
    if (pdrv != 0 || (disk_stat & STA_NOINIT)) return RES_NOTRDY;
    if (disk_stat & STA_PROTECT)               return RES_WRPRT;
    if (!count)                                return RES_PARERR;

    if (!(card_type & CT_BLOCK)) sector *= 512;

    DRESULT res = RES_ERROR;

    if (count == 1) {
        if (send_cmd(CMD24, sector) == 0 && xmit_datablock(buff, 0xFE)) {
            res = RES_OK;
        }
    } else {
        if (card_type & CT_SDC) send_cmd(ACMD23, count);

        if (send_cmd(CMD25, sector) == 0) {
            do {
                if (!xmit_datablock(buff, 0xFC)) break;
                buff += 512;
            } while (--count);

            if (!xmit_datablock(0, 0xFD)) count = 1; // Stop token
            if (!count) res = RES_OK;
        }
    }

    deselect();
    return res;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
    if (pdrv != 0 || (disk_stat & STA_NOINIT)) return RES_NOTRDY;

    DRESULT  res = RES_ERROR;
    uint8_t  n, csd[16];
    uint32_t csize;

    switch (cmd) {
    case CTRL_SYNC:
        if (select()) {
            deselect();
            res = RES_OK;
        }
        break;

    case GET_SECTOR_COUNT:
        if (send_cmd(CMD9, 0) == 0 && rcvr_datablock(csd, 16)) {
            if ((csd[0] >> 6) == 1) {
                // CSD v2 (SDHC/SDXC)
                csize = ((uint32_t)(csd[7] & 0x3F) << 16) |
                        ((uint32_t)csd[8]           <<  8) |
                        (uint32_t)csd[9];
                *(LBA_t*)buff = ((LBA_t)(csize + 1)) << 10;
            } else {
                // CSD v1
                n = (csd[5] & 0x0F) + ((csd[10] & 0x80) >> 7) +
                    ((csd[9]  & 0x03) << 1) + 2;
                csize = ((uint32_t)(csd[8] >> 6)) |
                        ((uint32_t)csd[7]  << 2)  |
                        ((uint32_t)(csd[6] & 0x03) << 10);
                *(LBA_t*)buff = (LBA_t)(csize + 1) << (n - 9);
            }
            res = RES_OK;
        }
        break;

    case GET_BLOCK_SIZE:
        if (card_type & CT_SD2) {
            // ACMD13 → SD Status
            if (send_cmd(ACMD13, 0) == 0) {
                spi_xchg(0xFF);
                if (rcvr_datablock(csd, 16)) {
                    for (n = 64 - 16; n; n--) spi_xchg(0xFF);
                    *(DWORD*)buff = 16UL << (csd[10] >> 4);
                    res = RES_OK;
                }
            }
        } else {
            if (send_cmd(CMD9, 0) == 0 && rcvr_datablock(csd, 16)) {
                if (card_type & CT_SD1) {
                    *(DWORD*)buff = (((csd[10] & 0x3F) << 1) +
                                    ((uint32_t)(csd[11] & 0x80) >> 7) + 1)
                                   << ((csd[13] >> 6) - 1);
                } else {
                    *(DWORD*)buff = ((uint32_t)((csd[10] & 0x7C) >> 2) + 1) *
                                   (((csd[11] & 0x03) << 3) +
                                    ((csd[11] & 0xE0) >> 5) + 1);
                }
                res = RES_OK;
            }
        }
        break;

    default:
        res = RES_PARERR;
        break;
    }

    deselect();
    return res;
}

/** Timestamp for FatFs (adjust if RTC present) */
DWORD get_fattime(void)
{
    // Without RTC: fixed to 2026-01-01 00:00:00
    return ((DWORD)(2026 - 1980) << 25) | // Year
           ((DWORD)1             << 21) | // Month
           ((DWORD)1             << 16);  // Date
}
