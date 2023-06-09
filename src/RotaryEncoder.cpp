// -----
// RotaryEncoder.cpp - Library for using rotary encoders.
// This class is implemented for use with the Arduino environment.
//
// Copyright (c) by Matthias Hertel, http://www.mathertel.de
//
// This work is licensed under a BSD 3-Clause style license,
// https://www.mathertel.de/License.aspx.
//
// More information on: http://www.mathertel.de/Arduino
// -----
// Changelog: see RotaryEncoder.h
// -----

#include "RotaryEncoder.h"
#include "Arduino.h"
#include "OptPin.h"


// ----- Initialization and Default Values -----

RotaryEncoder::RotaryEncoder(const RotaryEncoder::conf_t* cfg)
{
  // Remember Hardware Setup
  config = cfg;

  // Setup the input pins and turn on pullup resistor
  pin_t::SetDirection(&config->pin1, pin_t::Input);
  pin_t::Write(&config->pin1, true);
  pin_t::SetDirection(&config->pin2, pin_t::Input);
  pin_t::Write(&config->pin2, true);
  pin_t::SetDirection(&config->btn.pin, pin_t::Input);
  pin_t::Write(&config->btn.pin, true);

  // when not started in motion, the current state of the encoder should be 3
  _oldState = pin_t::Read(&config->pin1) | (pin_t::Read(&config->pin2) << 1);

  // start with position 0;
  //_position = 0;
  _positionExt = 0;
  _positionExtPrev = 0;

  PCICR |= config->pcint_group | config->btn.pcint_group;
  *(&PCMSK0 + config->pcint_group) |= config->pcint_mask;
  *(&PCMSK0 + config->btn.pcint_group) |= config->btn.pcint_mask;
} // RotaryEncoder()


uint16_t RotaryEncoder::getPosition()
{
  return _positionExt;
} // getPosition()


RotaryEncoder::Direction RotaryEncoder::getDirection()
{
  RotaryEncoder::Direction ret = Direction::NOROTATION;

  if (_positionExtPrev > _positionExt) {
    ret = Direction::COUNTERCLOCKWISE;
    _positionExtPrev = _positionExt;
  } else if (_positionExtPrev < _positionExt) {
    ret = Direction::CLOCKWISE;
    _positionExtPrev = _positionExt;
  } else {
    ret = Direction::NOROTATION;
    _positionExtPrev = _positionExt;
  }

  return ret;
}


void RotaryEncoder::setPosition(uint16_t newPosition)
{
    _positionExt = newPosition;
    _positionExtPrev = newPosition;
} // setPosition()


// End