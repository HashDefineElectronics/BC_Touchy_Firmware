/**
 * @file main.c
 * @author Ronald Sousa www.hashDefineElectronics.com
 * @date 13 Sep 2016
 */
#include "main.h"

// $[Generated Includes]
#include "cslib_config.h"
#include "cslib.h"
// [Generated Includes]$


/**
 * MUX_VALUE_ARRAY In inc/config/cslib_hwconfig.h should be set to this when updating the configurator
 */
// #define MUX_VALUE_ARRAY 0x0A, 0x0B, 0x0C, 0x02, 0x01, 0x09, 0x03, 0x06, 0x0D,


/**
 * define the LED pins
 */
SI_SBIT (LED1, SFR_P0, 0);
SI_SBIT (LED2, SFR_P2, 7);
SI_SBIT (LED3, SFR_P1, 6);
SI_SBIT (LED4, SFR_P1, 7);
SI_SBIT (LED5, SFR_P1, 0);

/**
 * @brief main program loop
 */
int main(void) {
	// Call hardware initialization routine
	enter_DefaultMode_from_RESET();
	Tick_Init();

	// enable all interrupts
	IE |= IE_EA__ENABLED | IE_ET2__ENABLED;

	while (1) {
// $[Generated Run-time code]
		// -----------------------------------------------------------------------------
		// If low power features are enabled, this will either put the device into a low
		// power state until it is time to take another scan, or put the device into a
		// low-power sleep mode if no touches are active
		// -----------------------------------------------------------------------------
		CSLIB_lowPowerUpdate();

		// -----------------------------------------------------------------------------
		// Performs all scanning and data structure updates
		// -----------------------------------------------------------------------------
		CSLIB_update();

// [Generated Run-time code]$
		circle_slider_main();

		if (CSLIB_isSensorDebounceActive(3)) {
			LED1 = false;
		}

		if (CSLIB_isSensorDebounceActive(4)) {
			LED2 = false;
		}

		if (CSLIB_isSensorDebounceActive(5)) {
			LED3 = false;
		}

		if (CSLIB_isSensorDebounceActive(6)) {
			LED4 = false;
		}

		if (CSLIB_isSensorDebounceActive(7)) {
			LED5 = false;
		}

		if (CSLIB_isSensorDebounceActive(8)) {
			LED1 = LED2 = LED3 = LED4 = LED5 = true;
			PCA0CPH0 = 0;
		}
	}
}
