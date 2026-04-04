#pragma once

#include "common.h"

/*
 * Gamecard slot config registers (from Decrypt9WIP protocol.c / 3dbrew CFG9)
 *
 * REG_CARDCONF (0x1000000C):
 *   bits 1:0  gamecard controller select (0=NTRCARD, 2=CTRCARD0, 3=CTRCARD1)
 *   bit  8    1=switch to SPICARD (save) interface
 *
 * REG_CARDCONF2 (0x10000010):
 *   slot power control - write 0x0C to power-cycle, then 0x4, then 0x8
 */
#define REG_CARDCONF    (*(vu32 *)0x1000000Cu)
#define REG_CARDCONF2   (*(vu8  *)0x10000010u)

/*
 * NTR card interface - ARM9-accessible MMIO at 0x10164000
 * (activated when REG_CARDCONF bits 1:0 == 0)
 *
 * References:
 *   Decrypt9WIP source/gamecart/protocol_ntr.h
 *   https://www.3dbrew.org/wiki/NTRCARD_Registers
 *   https://problemkaputt.de/gbatek.htm#dscartridgeprotocol
 */
#define NTRCARD_BASE    0x10164000u

#define NTRCARD_SPICNT  (*(vu16 *)(NTRCARD_BASE + 0x000))  /* SPI/MC control */
#define NTRCARD_SPIDATA (*(vu16 *)(NTRCARD_BASE + 0x002))  /* SPI data */
#define NTRCARD_ROMCNT  (*(vu32 *)(NTRCARD_BASE + 0x004))  /* ROM bus control / transfer trigger */
#define NTRCARD_CMD     ((vu8  *)(NTRCARD_BASE + 0x008))   /* 8-byte command (byte 0 sent first) */
#define NTRCARD_FIFO    (*(vu32 *)(NTRCARD_BASE + 0x01C))  /* read data FIFO */

/* SPICNT bits */
#define SPICNT_ENABLE   (1u << 15)  /* NTRCARD_CR1_ENABLE */
#define SPICNT_IRQ      (1u << 14)  /* NTRCARD_CR1_IRQ */

/*
 * ROMCNT bit definitions (from Decrypt9WIP protocol_ntr.h / libnds ndscard.h)
 *
 *  31     START/BUSY  write 1 to begin; reads 1 while busy  (NTRCARD_ACTIVATE / NTRCARD_BUSY)
 *  29     nRESET      1=normal, 0=card held in reset         (NTRCARD_nRESET)
 *  27     CLK_SLOW    1=4.2MHz, 0=6.7MHz                     (NTRCARD_CLK_SLOW)
 *  26:24  DATA_WORDS  0=0B, 1=4B, 2=8B, 3=512B, 4=1K,
 *                     5=2K, 6=4K, 7=4B                       (NTRCARD_BLK_SIZE)
 *  23     DATA_READY  read-only; 1=word ready in FIFO        (NTRCARD_DATA_READY)
 *  21:16  KEY1_GAP2   post-command dummy clocks              (NTRCARD_DELAY2)
 *  15     SEC_SEED    apply security seed                    (NTRCARD_SEC_SEED)
 *  12:0   KEY1_GAP1   pre-response dummy clocks              (NTRCARD_DELAY1)
 */
#define ROMCNT_KEY1_GAP1(n)  ((u32)(n) & 0x1FFF)
#define ROMCNT_KEY1_GAP2(n)  (((u32)(n) & 0x3F) << 16)
#define ROMCNT_SEC_SEED      (1u << 15)
#define ROMCNT_DATA_READY    (1u << 23)
#define ROMCNT_DATA_WORDS(n) (((u32)(n) & 0x7) << 24)
#define ROMCNT_CLK_SLOW      (1u << 27)
#define ROMCNT_nRESET        (1u << 29)
#define ROMCNT_START         (1u << 31)

void ntrcard_init(void);
u32 ntrcard_read_id(void);
