/******************************************************************************
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#include "efm8_device.h"
#include "boot.h"

// ----------------------------------------------------------------------------
// Initialize all hardware resources needed by the bootloader.
// ----------------------------------------------------------------------------
void boot_initDevice(void) 
{
  uint16_t count;

  // PCA0MD - PCA Mode
  // WDTE (Watchdog Timer Enable) = DISABLED (Disable Watchdog Timer.)
  PCA0MD &= ~PCA0MD_WDTE__BMASK;

  // CLKSEL - Clock Select
  // CLKSL (System Clock Source Select Bits) = HFOSC (Clock (SYSCLK)
  //     derived from the Internal High-Frequency Oscillator / 2.)
  CLKSEL = CLKSEL_CLKSL__HFOSC_DIV_2;

  // P0MDOUT - Port 0 Output Mode
  // P0.4 (UART0.TX) = PUSH_PULL
  P0MDOUT = P0MDOUT_B4__PUSH_PULL;

  // XBR0 - Port I/O Crossbar 0
  // URT0E (UART0 I/O Enable) = ENABLED (UART0 TX0, RX0 routed to Port pins P0.4 and P0.5.)
  XBR0 = XBR0_URT0E__ENABLED;

  // XBR1 - Port I/O Crossbar 1
  // WEAKPUD (Port I/O Weak Pullup Disable) = PULL_UPS_ENABLED (Weak Pullups enabled.)
  // XBARE (Crossbar Enable) = ENABLED (Crossbar enabled.)
  XBR1 = XBR1_WEAKPUD__PULL_UPS_ENABLED | XBR1_XBARE__ENABLED;

  // Auto Baud starts here ...
  // INT1 => active high, level sensitive interrupt on P0.5
  IT01CF = IT01CF_IN1PL__ACTIVE_HIGH | IT01CF_IN1SL__P0_5;

  // Timer1 => gated 16-bit counter using sysclk for the timebase
  CKCON0 = CKCON0_T1M__SYSCLK;
  TMOD = TMOD_GATE1__ENABLED | TMOD_T1M__MODE1;

  // Use Timer1 to measure the start bit time (LSB must be 1)
  TCON_TR1 = 1;
  while (P0_B5);
  while (!P0_B5);           // Wait for rising edge of the LSB
  TCON_TR1 = 0;

  // Divide measured count by 2 (Timer1 = 1 bit-time * 2)
  count  = TH1 << 8;
  count |= TL1;
  count >>= 1;

  // Configure Timer 1 for baud-rate generation using the measured value
  TH1 = 0x00 - (uint8_t)count;  // Timer counts up
  TMOD = TMOD_T1M__MODE2;       // 8-bit timer with auto-reload
  TCON_TR1 = 1;

  // Enable UART0 receiver
  SCON0_REN = 1;
}
