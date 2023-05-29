// -----
// RotaryEncoder.h - Library for using rotary encoders.
// This class is implemented for use with the Arduino environment.
//
// Copyright (c) by Matthias Hertel, http://www.mathertel.de
//
// This work is licensed under a BSD 3-Clause style license,
// https://www.mathertel.de/License.aspx.
//
// More information on: http://www.mathertel.de/Arduino
// -----
// 18.01.2014 created by Matthias Hertel
// 16.06.2019 pin initialization using INPUT_PULLUP
// 10.11.2020 Added the ability to obtain the encoder RPM
// 29.01.2021 Options for using rotary encoders with 2 state changes per latch.
// -----

#ifndef RotaryEncoder_h
#define RotaryEncoder_h

#include "Arduino.h"
#include "OptPin.h"

class RotaryEncoder
{
public:
  enum class Direction {
    NOROTATION = 0,
    CLOCKWISE = 1,
    COUNTERCLOCKWISE = -1
  };

  enum class LatchMode {
    FOUR3 = 1, // 4 steps, Latch at position 3 only (compatible to older versions)
    FOUR0 = 2, // 4 steps, Latch at position 0 (reverse wirings)
    TWO03 = 3  // 2 steps, Latch at position 0 and 3 
  };

  struct button_conf_t
  {
    uint8_t pcint_group;
    uint8_t pcint_mask;
    const pin_t pin;
  };
  struct conf_t
  {
    uint8_t pcint_group;
    uint8_t pcint_mask;
    const pin_t pin1;
    const pin_t pin2;
    RotaryEncoder::LatchMode mode;
    button_conf_t btn;
  };

  // ----- Constructor -----
  RotaryEncoder(const conf_t* cfg);

  // retrieve the current position
  unsigned long getPosition();

  // simple retrieve of the direction the knob was rotated last time. 0 = No rotation, 1 = Clockwise, -1 = Counter Clockwise
  Direction getDirection();

  // adjust the current position
  void setPosition(long newPosition);

  // call this function every some milliseconds or by using an interrupt for handling state changes of the rotary encoder.
  void tick(void);

  // Returns the time in milliseconds between the current observed
  unsigned long getMillisBetweenRotations() const;

  // Returns the RPM
  unsigned long getRPM();

private:
  const conf_t* config;

  volatile int8_t _oldState;

  volatile unsigned long _position;        // Internal position (4 times _positionExt)
  volatile unsigned long _positionExt;     // External position
  volatile unsigned long _positionExtPrev; // External position (used only for direction checking)

  unsigned long _positionExtTime;     // The time the last position change was detected.
  unsigned long _positionExtTimePrev; // The time the previous position change was detected.
};

#endif

// End