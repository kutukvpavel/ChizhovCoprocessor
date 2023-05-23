#pragma once

#include <Arduino.h>
#include "user.h"

#include "RotaryEncoder.h"
#include "OptPin.h"

#define MY_I2C_ADDR 0x08
#define MY_LED_ARDUINO_PIN 4
#define MY_LED_PORT PORTD
#define MY_LED_MASK _BV(PD4)
#define MY_PUMPS_NUM 3

#define MY_ENCODERS_NUM MY_PUMPS_NUM
namespace encoders
{
    const RotaryEncoder::conf_t config[MY_ENCODERS_NUM] = {
        {
            .pcint_group = PCIE0,
            .pcint_mask = _BV(PCINT4) | _BV(PCINT5),
            .pin1 = pin_t(DDRB, PB4),
            .pin2 = pin_t(DDRB, PB5),
            .mode = RotaryEncoder::LatchMode::FOUR3,
            .btn = {
                .pcint_group = PCIE1,
                .pcint_mask = _BV(PCINT8),
                .pin = pin_t(DDRC, PC0)
            }
        },
        {
            .pcint_group = PCIE0,
            .pcint_mask = _BV(PCINT2) | _BV(PCINT3),
            .pin1 = pin_t(DDRB, PB2),
            .pin2 = pin_t(DDRB, PB3),
            .mode = RotaryEncoder::LatchMode::FOUR3,
            .btn = {
                .pcint_group = PCIE1,
                .pcint_mask = _BV(PCINT8),
                .pin = pin_t(DDRC, PC1)
            }
        },
        {
            .pcint_group = PCIE0,
            .pcint_mask = _BV(PCINT0) | _BV(PCINT1),
            .pin1 = pin_t(DDRB, PB0),
            .pin2 = pin_t(DDRB, PB1),
            .mode = RotaryEncoder::LatchMode::FOUR3,
            .btn = {
                .pcint_group = PCIE1,
                .pcint_mask = _BV(PCINT8),
                .pin = pin_t(DDRC, PC2)
            }
        }
    };
} // namespace encoders

#define MY_DRIVES_NUM MY_PUMPS_NUM
namespace drv
{
    const uint8_t station_addrs[MY_PUMPS_NUM] = { 0xE0, 0xE0, 0xE0 };
    const pin_t pin_addr_a(DDRD, PD5);
    const pin_t pin_addr_b(DDRD, PD6);
    const pin_t pin_addr_c(DDRD, PD7);
} // namespace drv

#define MY_POTS_NUM 1
namespace pots
{
    const uint8_t averaging_len = 10;
    const uint8_t manual_override_idx = 0;
    const uint8_t adc_channels[MY_POTS_NUM] = { 4 };
} // namespace pots