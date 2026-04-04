#include "ntrcard.h"

extern void waitcycles(u32 us);

/*
 * Power-cycle the cartridge slot.
 * Mirrors Decrypt9WIP ResetCartSlot() in source/gamecart/protocol.c.
 */
static void ntrcard_reset_slot(void)
{
	REG_CARDCONF2 = 0x0C;
	REG_CARDCONF &= ~0x3u;

	if (REG_CARDCONF2 == 0x0C) {
		while (REG_CARDCONF2 != 0);
	}
	if (REG_CARDCONF2 != 0)
		return;

	REG_CARDCONF2 = 0x4;
	while (REG_CARDCONF2 != 0x4);

	REG_CARDCONF2 = 0x8;
	while (REG_CARDCONF2 != 0x8);
}

/*
 * Send the 0x9F dummy command and drain the response.
 * This returns the card to a known state after reset.
 * Mirrors Decrypt9WIP cardReset() in source/gamecart/card_ntr.c.
 */
static void ntrcard_dummy_reset(void)
{
	NTRCARD_SPICNT = SPICNT_ENABLE | SPICNT_IRQ;

	NTRCARD_CMD[0] = 0x9F;
	NTRCARD_CMD[1] = 0; NTRCARD_CMD[2] = 0; NTRCARD_CMD[3] = 0;
	NTRCARD_CMD[4] = 0; NTRCARD_CMD[5] = 0; NTRCARD_CMD[6] = 0; NTRCARD_CMD[7] = 0;

	/* BLK_SIZE(5)=2048 bytes, CLK_SLOW, DELAY2=0x18 */
	NTRCARD_ROMCNT = ROMCNT_START | ROMCNT_nRESET | ROMCNT_CLK_SLOW |
	                 ROMCNT_DATA_WORDS(5) | ROMCNT_KEY1_GAP2(0x18);

	do {
		if (NTRCARD_ROMCNT & ROMCNT_DATA_READY) {
			(void)NTRCARD_FIFO;
		}
	} while (NTRCARD_ROMCNT & ROMCNT_START);

	waitcycles(0xF000);
}

/*
 * Full NTR card initialisation sequence.
 * Mirrors Decrypt9WIP Cart_Init() in source/gamecart/protocol.c.
 */
void ntrcard_init(void)
{
	ntrcard_reset_slot();
	waitcycles(0x40000);

	/* Switch to NTRCARD controller - mirrors SwitchToNTRCARD() */
	NTRCARD_ROMCNT = ROMCNT_nRESET;
	REG_CARDCONF &= ~0x3u;    /* bits 1:0 = 0 → NTRCARD */
	REG_CARDCONF &= ~0x100u;  /* bit 8 = 0 → ROM interface (not SPI) */
	NTRCARD_SPICNT = SPICNT_ENABLE;
	waitcycles(0x40000);

	NTRCARD_ROMCNT = 0;
	NTRCARD_SPICNT &= 0xFF;   /* clear high byte of SPICNT */
	waitcycles(0x40000);

	NTRCARD_SPICNT |= SPICNT_ENABLE | SPICNT_IRQ;
	NTRCARD_ROMCNT = ROMCNT_nRESET | ROMCNT_SEC_SEED;
	while (NTRCARD_ROMCNT & ROMCNT_START);

	ntrcard_dummy_reset();
	waitcycles(0x40000);
}

/*
 * Send NTR_CMD_ID_NORMAL_READ_ID (0x90) and return the 4-byte chip ID.
 * Mirrors Decrypt9WIP cardReadID() / cardWriteAndRead() in card_ntr.c.
 */
u32 ntrcard_read_id(void)
{
	NTRCARD_SPICNT = SPICNT_ENABLE | SPICNT_IRQ;

	NTRCARD_CMD[0] = 0x90;
	NTRCARD_CMD[1] = 0; NTRCARD_CMD[2] = 0; NTRCARD_CMD[3] = 0;
	NTRCARD_CMD[4] = 0; NTRCARD_CMD[5] = 0; NTRCARD_CMD[6] = 0; NTRCARD_CMD[7] = 0;

	/* BLK_SIZE(7)=4B, CLK_SLOW, >=4 latency cycles (DSpico spec) */
	NTRCARD_ROMCNT = ROMCNT_START | ROMCNT_nRESET | ROMCNT_CLK_SLOW |
	                 ROMCNT_DATA_WORDS(7) | ROMCNT_KEY1_GAP2(0x18) |
	                 ROMCNT_KEY1_GAP1(4);

	while (!(NTRCARD_ROMCNT & ROMCNT_DATA_READY));
	return NTRCARD_FIFO;
}
