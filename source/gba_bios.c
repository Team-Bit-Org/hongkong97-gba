#include "gba_bios.h"

void vblank_intr_wait()
{
  // 0x5 is VBlankIntrWait in the function table.
  asm volatile("swi 0x05");
}

ARM void interruptHandler()
{
	REG_IF = INT_VBLANK;
	REG_IFBIOS |= INT_VBLANK;
}

void register_vblank_isr()
{
	REG_IME = 0x00;
	REG_INTERRUPT = (fnptr)interruptHandler;
	REG_DISPSTAT |= DSTAT_VBL_IRQ;
	REG_IE |= INT_VBLANK;
	REG_IME = 0x01;
}