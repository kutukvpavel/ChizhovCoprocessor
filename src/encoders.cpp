#include "encoders.h"

#include <avr/interrupt.h>

#include "RotaryEncoder.h"
#include "my_conf.h"

namespace encoders
{
    static const pin_t button_pins[MY_ENCODERS_NUM] = {
        pin_t(DDRC, PINC0),
        pin_t(DDRC, PINC1),
        pin_t(DDRC, PINC2)
    };
    static bool button_initial[MY_ENCODERS_NUM];

    static RotaryEncoder* instances[MY_ENCODERS_NUM];
    static bool buttons[MY_ENCODERS_NUM] = { 0 };

    void init()
    {
        for (size_t i = 0; i < MY_ENCODERS_NUM; i++)
        {
            instances[i] = new RotaryEncoder(&(config[i]));
            pin_t::SetDirection(&(button_pins[i]), pin_t::Input);
            pin_t::Write(&(button_pins[i]), true); //Pull-up
            delayMicroseconds(10);
            button_initial[i] = pin_t::Read(&(button_pins[i]));
        }
    }

    uint16_t get_position(uint8_t i)
    {
        return instances[i]->getPosition();
    }
    void poll_buttons()
    {
        const uint32_t poll_interval = 15; //ms
        const uint8_t debounce_intervals = 4;

        static uint32_t last_poll = 0;
        static uint8_t debounce[MY_ENCODERS_NUM] = { 0 };
        static bool buffer = false;

        if ((millis() - last_poll) < poll_interval) return;

        for (size_t i = 0; i < MY_ENCODERS_NUM; i++)
        {
            buffer = (pin_t::Read(&(button_pins[i])) != button_initial[i]);
            if (buffer != buttons[i]) debounce[i]++;
            else debounce[i] = 0;
            if (debounce[i] >= debounce_intervals)
            {
                debounce[i] = 0;
                buttons[i] = buffer;
            }
        }
    }
    bool get_button_pressed(uint8_t i)
    {
        return buttons[i];    
    }
} // namespace encoders

#pragma GCC push_options
#pragma GCC optimize ("unroll-loops")
ISR(PCINT0_vect) //Cross-check with RotaryEncoder config (PCINT groups, currently only PCIE0 is used => PCINT0)
{
    for (uint8_t i = 0; i < MY_ENCODERS_NUM; i++)
    {
        if (encoders::instances[i]->tick()) return;
    }
}
#pragma GCC pop_options