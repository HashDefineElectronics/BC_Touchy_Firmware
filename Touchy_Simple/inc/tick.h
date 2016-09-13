/**
 * @file tick.h
 * @author Ronald Sousa www.hashDefineElectronics.com
 * @date 13 Sep 2016
 */
#ifndef __TICK_H_
#define __TICK_H_

	void Tick_Init(void);
	uint16_t Tick_GetCount();
	void Tick_Wait(uint16_t ms);

#endif
