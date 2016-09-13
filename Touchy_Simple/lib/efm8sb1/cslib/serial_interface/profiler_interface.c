/**************************************************************************//**
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#include "cslib_config.h"
#include "profiler_interface.h"
#include "cslib.h"
#include "comm_routines.h"
//#include "SliderLibrary.h"
//#include "SliderConfig.h"
#include "cslib_sensor_descriptors.h"
//#include "SliderDescriptors.h"

void printOutput(uint16_t offset, uint8_t bytes);




// ? #if OUTPUT_MODE == FULL_OUTPUT_RX_FROM_SENSOR

#define HEADER_TYPE_COUNT 11

idata HeaderStruct_t headerEntries[HEADER_TYPE_COUNT] = {

{"BASELINE", DEF_NUM_SENSORS},
{"RAW", DEF_NUM_SENSORS},
{"PROCESS", DEF_NUM_SENSORS},
{"SINGACT", DEF_NUM_SENSORS},
{"DEBACT", DEF_NUM_SENSORS},
{"TDELTA", DEF_NUM_SENSORS},
{"EXPVAL", DEF_NUM_SENSORS},
{"NOISE", DEF_NUM_SENSORS},

//{"SLIDER", DEF_NUM_SLIDERS},
{"NOISEEST", 1},
{"ACTTHR", 1},
{"INACTTHR", 1}

};

idata uint8_t offsetArray[HEADER_TYPE_COUNT];


//-----------------------------------------------------------------------------
// Local variables and macros
//-----------------------------------------------------------------------------



uint8_t needANewLine = 0;                 // Flag to tell output to add a
                                       // carriage return

// These macros are called throughout the file to add carriage returns
// to the output byte stream as needed
#define NEED_NEWLINE() needANewLine = 1
#define CLEAR_NEWLINE() needANewLine = 0

// This flag is a one-shot that will output the defined header string
// at the start of the string.  This example requires a device reset
// before the header is ever sent again.
uint8_t sendHeader = 1;

//-----------------------------------------------------------------------------
// Local function prototypes
//-----------------------------------------------------------------------------

void printHeader(void);               // Generates and outputs a header
                                       // describing the data in the stream


//-----------------------------------------------------------------------------
// CSLIB_commUpdate()
//-----------------------------------------------------------------------------
//
// Exposed top-level debug output function.  Outputs one line of data output
// for each call.  Output controls are compile-time switches found in
// profiler_interface.h.  Function prints all of defined variable types in
// sequence before printing the next defined variable type.  For exmample,
// a configuration to print raw and baseline data for a system with two
// channels would print:
// <RAW_0> <RAW_1> <BASELINE_0> <BASELINE_1>
//

void CSLIB_commUpdate(void)
{
   uint16_t SI_SEG_XDATA value;

   // This is set during device initialization as a one-shot
   if(sendHeader == 1)
   {
      printHeader();
      sendHeader = 0;
   }

#if OUTPUT_MODE == FULL_OUTPUT_RX_FROM_SENSOR


   printBase = (uint16_t)&CSLIB_node[0];
   printSize = sizeof(CSLIB_node[0]);
   printCount = DEF_NUM_SENSORS;

   //"BASELINE",
   printOutput(offsetArray[0], 2);
   //"RAW",
   printOutput(offsetArray[1], 2);
   //"PROCESS",
   printOutput(offsetArray[2], 2);
   //"SINGACT",
   printOutputSingAct(offsetArray[3], 1);
   //"DEBACT",
   printOutputDebAct(offsetArray[4], 1);
   //"TDELTA",
   printOutputTDelta(offsetArray[5], 1);
   //"NOISE",
  // printOutput(offsetArray[6], 1);
   //"EXPVAL",
   printOutput(offsetArray[7], 2);
   /*
   //"SLIDER"
   printBase = &Slider;
   printSize = sizeof(Slider[0]);
   */
   value = 0;
   printSize = 1;
   printBase = (uint16_t)&value;
   printCount = DEF_NUM_SENSORS;
   printOutput(0, 1);
   //"noise est"
   value = (uint16_t)CSLIB_systemNoiseAverage;
   printSize = 2;
   printBase = (uint16_t)&value;
   printCount = 1;
   printOutput(0, 2);

   //"ACT_THR"
   value = (uint16_t)CSLIB_activeSensorDelta;
   printSize = 2;
   printBase = (uint16_t)&value;
   printCount = 1;
   printOutput(0, 2);

   //"INACT_THR"
   value = (uint16_t)CSLIB_inactiveSensorDelta;
   printSize = 2;
   printBase = (uint16_t)&value;
   printCount = 1;
   printOutput(0, 2);

   outputNewLine();

#endif



}




//-----------------------------------------------------------------------------
// printHeader
//-----------------------------------------------------------------------------
//
// Function defines data bytes to follow with byte-by-byte designators.
//

void calculateOffsets(void)
{
/*
"BASELINE",
"RAW",
"PROCESS",
"SINGACT",
"DEBACT",
"TDELTA",
"NOISE",
"EXPVAL",
"NOISEEST"
*/
#if OUTPUT_MODE == FULL_OUTPUT_RX_FROM_SENSOR
   uint16_t base = &CSLIB_node;
   // "BASELINE",
   offsetArray[0] = (uint16_t)&(CSLIB_node[0].currentBaseline) - base;
   // "RAW",
   offsetArray[1] = (uint16_t)&(CSLIB_node[0].rawBuffer[0]) - base;
   // "PROCESS",
   offsetArray[2] = (uint16_t)&(CSLIB_node[0].processBuffer[0]) - base;
   // "SINGACT",
   offsetArray[3] = (uint16_t)&(CSLIB_node[0].activeIndicator) - base;
   // "DEBACT",
   offsetArray[4] = (uint16_t)&(CSLIB_node[0].activeIndicator) - base;
   // "TDELTA",
   offsetArray[5] = (uint16_t)&(CSLIB_node[0].touchDeltaDiv16) - base;
   // "NOISE",
   offsetArray[6] = (uint16_t)&(CSLIB_node[0].touchDeltaDiv16) - base;
   // "EXPVAL",
   offsetArray[7] = (uint16_t)&(CSLIB_node[0].expValue[0]) - base;
   // "NOISEEST"
   offsetArray[8] = 0;
   offsetArray[9] = (uint16_t)&(CSLIB_node[0].debounceCounts) - base;
   offsetArray[10] = (uint16_t)&(CSLIB_node[0].debounceCounts) - base;


#endif
}

void printHeader(void)
{
   uint8_t index;
   outputNewLine();
#ifdef HAS_SENSOR_DESCRIPTORS
   outputsensorDescriptors();
#endif
#ifdef HAS_SLIDER_DESCRIPTORS
   OutputSliderDescriptors();
#endif
   outputBeginHeader();

#if OUTPUT_MODE == FULL_OUTPUT_RX_FROM_SENSOR
   for(index = 0; index < HEADER_TYPE_COUNT; index++)
   {
      outputHeaderCount(headerEntries[index]);
      outputBreak();
   }
#endif

#if OUTPUT_MODE == FULL_OUTPUT_RX_FROM_FILE
   for(index = 0; index < HEADER_TYPE_COUNT; index++)
   {
      outputHeaderCount(HeaderStrings[index], NUM_SENSORS);
      outputBreak();
   }
#endif
   outputNewLine();
   calculateOffsets();


}

