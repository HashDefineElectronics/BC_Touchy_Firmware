/**************************************************************************//**
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#ifndef _LOW_POWER_CONFIG_H
#define _LOW_POWER_CONFIG_H


// Functions which must be defined with implementation-specific
// responsibilities.  These are called by LowPowerRoutines.c
void configureSensorForSleepMode(void);
void configureTimerForSleepMode(void);
void configureTimerForActiveMode(void);
void enterLowPowerState(void);
void checkTimer(void);
extern xdata uint8_t timerTick;


#endif