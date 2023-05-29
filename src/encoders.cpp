#include "encoders.h"

#include <avr/interrupt.h>

#include "RotaryEncoder.h"
#include "my_conf.h"

namespace encoders
{
    RotaryEncoder* instances[MY_ENCODERS_NUM];

    void init()
    {
        for (size_t i = 0; i < MY_ENCODERS_NUM; i++)
        {
            instances[i] = new RotaryEncoder(&(config[i]));
        }
    }

    unsigned long get_position(uint8_t i)
    {
        return instances[i]->getPosition();
    }
} // namespace encoders
