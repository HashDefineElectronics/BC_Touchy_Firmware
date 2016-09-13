/**
 * @file tick.c
 * @author Ronald Sousa www.hashDefineElectronics.com
 * @date 13 Sep 2016
 */
#include "main.h"

/**
 * holds the current tick valus since the micro powered up
 */
static uint16_t Ticks = 0;

/**
 * @brief Return the system up time in millisecond
 *
 * @return  Number of milliseconds since system start.
 */
static uint16_t Tick_GetCount(void) {
    uint16_t ticks;

    // Disable Timer 3 interrupts
    EIE1 &= ~EIE1_ET3__BMASK;

    ticks = Ticks;

    // Enable Timer 3 interrupts
    EIE1 |= EIE1_ET3__BMASK;

    return ticks;
}

/**
 * @brief Wait the specified number of milliseconds
 *
 * @param ms The number of milliseconds to wait
 *
 * @note this function is a blocking type
 */
void Tick_Wait(uint16_t ms) {
    uint16_t ticks = Tick_GetCount();

    while ((Tick_GetCount() - ticks) < ms);
}

/**
 * @brief configure the timers and interrupt to generate the
 * tick value as 1ms interval
 */
void Tick_Init(void) {
	uint8_t TMR3CN0_TR3_save;
	TMR3CN0_TR3_save = TMR3CN0 & TMR3CN0_TR3__BMASK;
	TMR3CN0 &= ~(TMR3CN0_TR3__BMASK);
	TMR3H = (0xF9 << TMR3H_TMR3H__SHIFT);
	TMR3L = (0x7D << TMR3L_TMR3L__SHIFT);
	TMR3RLH = (0xF9 << TMR3RLH_TMR3RLH__SHIFT);
	TMR3RLL = (0x7D << TMR3RLL_TMR3RLL__SHIFT);
	TMR3CN0 |= TMR3CN0_TR3__RUN;
	TMR3CN0 |= TMR3CN0_TR3_save;

	EIE1 |= EIE1_ET3__ENABLED;
}

/**
 * @brief timer 3 interrupt
 */
SI_INTERRUPT (TIMER3_ISR, TIMER3_IRQn) {
    // Overflows every 1 ms
    TMR3CN0 &= ~TMR3CN0_TF3H__BMASK;

    Ticks++;
}
