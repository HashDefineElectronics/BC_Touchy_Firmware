/**************************************************************************//**
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/
#include <SI_EFM8SB1_Register_Enums.h>                  // SFR declarations
#include "circle_slider.h"
#include "cslib_config.h"
#include "cslib.h"


void SetLedIntensity(uint8_t intensity) {
    if (intensity == 0)
    {
        PCA0CPM0 &= ~PCA0CPM0_ECOM__ENABLED;
    }
    else
    {
        PCA0CPH0 = 255-intensity;
    }
}

// Given an angle, normalize the angle
// to between 0 and 360 degrees [0, 360)
void NormalizeAngle(uint16_t* angle) {
    while (*angle >= 360)
    {
        *angle -= 360;
    }
}

// Update the LED brightness based on angle
// angle [0, 359] => intensity [0, 255]
void UpdateLed(uint16_t angle) {
  static lastIntensity = 0;

  // 256 / 360 reduces to 32 / 45
  uint8_t intensity = angle * 32 / 45;

  if (intensity != lastIntensity)
  {
    SetLedIntensity(intensity);
    lastIntensity = intensity;
  }
}

bool IsTouchQualified(void) {
  uint8_t sensor_index;
  uint16_t touch_sum = 0;

  // Only update the slider if at least one of the CS0 channels is active
  if (CSLIB_isSensorDebounceActive(0) ||
      CSLIB_isSensorDebounceActive(1) ||
      CSLIB_isSensorDebounceActive(2))
  {
    // For each sensor, calculate the delta between the current
    // capacitance value and baseline capacitance value, or delta
    for (sensor_index = 0; sensor_index < 3; sensor_index++)
    {
        if (CSLIB_node[sensor_index].processBuffer[0] >= CSLIB_node[sensor_index].currentBaseline)
        {
            touch_sum += ((CSLIB_node[sensor_index].processBuffer[0] - CSLIB_node[sensor_index].currentBaseline) >> 8);
        }
    }

    // Sum of touch deltas must reach a minimum threshold
    // before updating circle slider position.
    //
    // This helps reduce jumpiness when the touch
    // is being released due to non-linear capacitance
    // response
    if (touch_sum > MIN_SUM_TOUCH)
    {
      return true;
    }
  }

  return false;
}

// Perform the centroid algorithm to determine the wheel angle position
// (0 degrees is 12 o'clock, 90 degrees is 3 o'clock)
//
// Sensor orientation:
//
// CS1 CS2
//   CS0
//
uint16_t CalculatePosition() {
    uint8_t sensor_index;
    uint16_t angle = 0;
    uint16_t deltas[3];
    uint16_t weights[3];
    uint16_t offset;

    // For each sensor, calculate the delta between the current
    // capacitance value and baseline capacitance value, or delta
    for (sensor_index = 0; sensor_index < 3; sensor_index++)
    {
        if (CSLIB_node[sensor_index].processBuffer[0] < CSLIB_node[sensor_index].currentBaseline)
        {
            deltas[sensor_index] = 0;
        }
        else
        {
            deltas[sensor_index] = (CSLIB_node[sensor_index].processBuffer[0] - CSLIB_node[sensor_index].currentBaseline) >> 8;
        }
    }

    // CS0 touch delta is highest, perform weighted linear slider
    // binning algorithm to assign a value between 20 - 60
    if (deltas[0] >= deltas[2] &&
        deltas[0] >= deltas[1])
    {
        // Binned weights by sensor position:
        // CS1:60 CS2:20
        //    CS0:40
        weights[0] = 40;
        weights[1] = 60;
        weights[2] = 20;

        // An average weighted value of 20 equals 60 degrees
        offset = 60;
    }
    // CS2 touch delta is highest, perform weighted linear slider
    // binning algorithm to assign a value between 20 - 60
    else if (deltas[2] >= deltas[0] &&
             deltas[2] >= deltas[1])
    {
        // Binned weights by sensor position:
        // CS1:20 CS2:40
        //    CS0:60
        weights[0] = 60;
        weights[1] = 20;
        weights[2] = 40;

        // An average weighted value of 20 equals -60 degrees, or 300 degrees
        offset = 300;
    }
    // CS1 touch delta is highest, perform weighted linear slider
    // binning algorithm to assign a value between 20 - 60
    else
    {
        // Binned weights by sensor position:
        // CS1:40 CS2:60
        //   CS0:20
        weights[0] = 20;
        weights[1] = 40;
        weights[2] = 60;

        // An average weighted value of 20 equals 180 degrees
        offset = 180;
    }

    // Calculate the angle using binned weights and offset from above

    // Apply calculated binned weights to get a value between [20, 60]
    angle = (deltas[0] * weights[0] + deltas[1] * weights[1] + deltas[2] * weights[2]) / (deltas[0] + deltas[1] + deltas[2]);

    // Convert binned value from [20, 60] to [0, 240]
    angle = (6 * angle) - 120;

    // Apply offset
    angle += offset;

    // Normalize angle between [0, 360)
    NormalizeAngle(&angle);

    return angle;
}

void circle_slider_main() {
  uint16_t angle;

  // Check if sum of sensors is above a minimum threshold
  // for better touch release behaviour
  if (IsTouchQualified())
  {
    // Perform the centroid algorithm to determine the wheel angle position
    // (0 degrees is 12 o'clock, 90 degrees is 3 o'clock)
    angle = CalculatePosition();

    // Update the LED brightness based on angle
    UpdateLed(angle);
  }
}
