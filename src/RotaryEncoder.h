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
    button_conf_t btn;
  };

  // ----- Constructor -----
  RotaryEncoder(const conf_t* cfg);

  // retrieve the current position
  uint16_t getPosition();

  // simple retrieve of the direction the knob was rotated last time. 0 = No rotation, 1 = Clockwise, -1 = Counter Clockwise
  Direction getDirection();

  // adjust the current position
  void setPosition(uint16_t newPosition);

  // call this function every some milliseconds or by using an interrupt for handling state changes of the rotary encoder.
  inline bool RotaryEncoder::tick()
  {
    // The array holds the values �1 for the entries where a position was decremented,
    // a 1 for the entries where the position was incremented
    // and 0 in all the other (no change or not valid) cases.

    // positions: [3] 1 0 2 [3] 1 0 2 [3]
    // [3] is the positions where my rotary switch detends
    // ==> right, count up
    // <== left,  count down
    static const int8_t KNOBDIR[] = {
      0, -1, 1, 0,
      1, 0, 0, -1,
      -1, 0, 0, 1,
      0, 1, -1, 0};

    uint8_t thisState = static_cast<uint8_t>(pin_t::Read(&config->pin1)) | 
      static_cast<uint8_t>(static_cast<uint8_t>(pin_t::Read(&config->pin2)) << 1_ui8);

    if (_oldState == thisState) return false;

    _positionExt += KNOBDIR[static_cast<uint8_t>(thisState | static_cast<uint8_t>(_oldState << 2_ui8))];
    _oldState = thisState;
    
    return true;
  } // tick()

private:
  const conf_t* config;

  volatile uint8_t _oldState;

  volatile uint16_t _positionExt;     // External position
  volatile uint16_t _positionExtPrev; // External position (used only for direction checking)
};

#endif

// End