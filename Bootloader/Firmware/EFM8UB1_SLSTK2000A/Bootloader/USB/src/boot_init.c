/******************************************************************************
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#include "efm8_device.h"
#include "boot.h"
#include "USB_main.h"

// ----------------------------------------------------------------------------
// Initialize all hardware resources needed by the bootloader.
// ----------------------------------------------------------------------------
void boot_initDevice(void) 
{
  //Disable Watchdog with key sequence
  WDTCN = 0xDE;
  WDTCN = 0xAD;

  // Enable VDD monitor and set it as a reset source
  VDM0CN |= VDM0CN_VDMEN__ENABLED;
  RSTSRC = RSTSRC_PORSF__SET;

  // PFE0CN - Prefetch Engine Control
  // PFEN (Prefetch Enable) = ENABLED (Enable the prefetch engine (SYSCLK > 25 MHz).)
  // FLRT (Flash Read Timing) = SYSCLK_BELOW_50_MHZ (SYSCLK < 50 MHz.)
  SET_SFRPAGE(PG2_PAGE);
  PFE0CN = PFE0CN_PFEN__ENABLED | PFE0CN_FLRT__SYSCLK_BELOW_50_MHZ;

  // CLKSEL - Clock Select
  // CLKSL (Clock Source Select) = HFOSC1 (Clock derived from the Internal
  //     High Frequency Oscillator 1.)
  // CLKDIV (Clock Source Divider) = SYSCLK_DIV_1 (SYSCLK is equal to
  //     selected clock source divided by 1.)
  CLKSEL = CLKSEL_CLKSL__HFOSC0 | CLKSEL_CLKDIV__SYSCLK_DIV_1;
  while(CLKSEL & CLKSEL_DIVRDY__BMASK == CLKSEL_DIVRDY__NOT_READY);
  CLKSEL = CLKSEL_CLKSL__HFOSC1 | CLKSEL_CLKDIV__SYSCLK_DIV_1;

  // Source the USB clock from HFOSC1 (48 MHz)
  SET_SFRPAGE(USB0_PAGE);
  USB0CF = USB0CF_USBCLK__HFOSC1;

  // Initialize the USB driver
  USB_initModule();
}
