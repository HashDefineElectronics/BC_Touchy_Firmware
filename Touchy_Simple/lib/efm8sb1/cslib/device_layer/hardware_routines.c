/**************************************************************************//**
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#include "cslib_config.h"
#include "cslib.h"

#include "hardware_routines.h"
#include "cslib_hwconfig.h"
#include "SI_EFM8SB1_Defs.h"


idata TechSpecificStruct_t CSLIB_techSpec[DEF_NUM_SENSORS];
#define disableSleepAndStall 0
//-----------------------------------------------------------------------------
// Local function prototypes
//-----------------------------------------------------------------------------
void configureCS0ActiveMode(void);
void configurePortsActiveMode(void);
//-----------------------------------------------------------------------------
// setMux
//-----------------------------------------------------------------------------
//
// Low level access to the CS0MUX register.  This is called by scanSensor().
//
void setMux(uint8_t muxValue)
{
   CS0MX = muxValue;
}

//-----------------------------------------------------------------------------
// setGain
//-----------------------------------------------------------------------------
//
// Low level access to the gain bits.  This is called by scanSensor().
//
void setGain(uint8_t gainValue)
{
   CS0MD1 = 0x07 & gainValue;         // clear all bits except gain bits
}

//-----------------------------------------------------------------------------
// readGain
//-----------------------------------------------------------------------------
//
// Returns the gain bits of the sensor.
//
uint8_t readGain(void)
{
   return CS0MD1 & 0x07;
}

//-----------------------------------------------------------------------------
// readMux
//-----------------------------------------------------------------------------
//
// Returns the mux value from the sensor peripheral.
//
uint8_t readMux(void)
{
   return CS0MX;
}

//-----------------------------------------------------------------------------
// setAccumulation
//-----------------------------------------------------------------------------
//
// Sets the accumulation bits of the sensor register.
//
void setAccumulation(uint8_t accumulation)
{
   CS0CF = 0x07 & accumulation;        // Clear all bits except accumulation
}


//-----------------------------------------------------------------------------
// executeConversion
//-----------------------------------------------------------------------------
//
// Assumes that the performance characteristics of the sensor have already
// been configured.  Enables the sensor, starts a scan, blocks until
// the scan is complete.
// Note that this version of executeConversion() allows for a retrieval
// of sensor data from the serial port through a call to getU16().  This
// is a compile-time switch.
//
uint16_t executeConversion(void)
{

      SI_UU16_t scanResult;

      CS0CN0 = 0x88;                       // Enable CS0, Enable Digital Comparator

      CS0CN0 &= ~0x20;                     // Clear the CS0 INT flag

      CS0CN0 |= 0x10;                      // Set CS0BUSY to begin conversion

      if(disableSleepAndStall == 0)
      {
         PMU0FL = 0x01;
         PMU0CF = 0x40;				  // Enter suspend until conversion completes
      }

      while (!(CS0CN0 & 0x20));            // Wait in foreground


      scanResult.u8[MSB] = CS0DH;         // Read Result
      scanResult.u8[LSB] = CS0DL;

      CS0CN0 = 0x00;                       // Disable CS0

      return scanResult.u16;


}




//-----------------------------------------------------------------------------
// configureSensorForActiveMode
//-----------------------------------------------------------------------------
//
// This is a top-level call to configure the sensor to its operational state
// during active mode.
//
void configureSensorForActiveMode(void)
{
   configurePortsActiveMode();
   configureCS0ActiveMode();
}

//-----------------------------------------------------------------------------
// scanSensor
//-----------------------------------------------------------------------------
//
// This is a top-level function that assumes there is a sensor node struct
// with CS0-related controls.  This is called by the library to execute
// one scan of one sensor node.  Note that the sensor output is returned
// and not saved to buffers in this routine.  Saving is the responsibility
// of the library routines.
//
uint16_t scanSensor(uint8_t nodeIndex)
{
   setMux(CSLIB_techSpec[nodeIndex].mux);
   setGain(CSLIB_techSpec[nodeIndex].gain);
   setAccumulation(CSLIB_techSpec[nodeIndex].accumulation);
   return executeConversion();
}




//-----------------------------------------------------------------------------
// Local hardware access functions
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// configureCS0ActiveMode
//-----------------------------------------------------------------------------
//
// This is a low-level local routine to handle configuration of the CS0 block.
//
void configureCS0ActiveMode(void)
{

	CS0CF = 0x00;                         // MODE: CS0BUSY
   CS0MD2 &= 0xC0;                       // 12-bit mode
   CS0MD2 |= 0x40;
   CS0THH = 0;
   CS0THL = 0;

   CS0CN0 = 0x88;                       // Enable CS0, Enable Digital Comparator

   CS0CN0 &= ~0x20;

   CS0CN0 |= 0x10;                      // Set CS0BUSY to begin conversion
   while (!(CS0CN0 & 0x20));            // Wait in foreground

   CS0CN0 = 0x00;                       // Disable CS0

}


//-----------------------------------------------------------------------------
// configurePortsActiveMode
//-----------------------------------------------------------------------------
//
// This is a low-level local routine to handle configuration of the ports
// for active mode.
//
void configurePortsActiveMode(void)
{
   P0MDIN |= SLEEP_MODE_MASK_P0;
   P1MDIN |= SLEEP_MODE_MASK_P1;

   P0MDOUT |= ACTIVE_MODE_MASK_P0;
   P0 &= ~ACTIVE_MODE_MASK_P0;
   P0MDIN &=~ACTIVE_MODE_MASK_P0;            // Set P1.0 and 1.1 to analog
   P0MDOUT &= ~ACTIVE_MODE_MASK_P0;
   P0 |= ACTIVE_MODE_MASK_P0;

   P1MDOUT |= ACTIVE_MODE_MASK_P1;
   P1 &= ~ACTIVE_MODE_MASK_P1;
   P1MDIN &=~ACTIVE_MODE_MASK_P1;            // Set P1.0 and 1.1 to analog
   P1MDOUT &= ~ACTIVE_MODE_MASK_P1;
   P1 |= ACTIVE_MODE_MASK_P1;


}



void nodeInit(uint8_t sensorIndex)
{
  CSLIB_techSpec[sensorIndex].gain = CSLIB_gainValues[sensorIndex];
  CSLIB_techSpec[sensorIndex].mux = CSLIB_muxValues[sensorIndex];
  CSLIB_techSpec[sensorIndex].accumulation = CSLIB_accumulationValues[sensorIndex];
}
