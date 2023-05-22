#include "pots.h"

#include "my_conf.h"
#include "OptPin.h"

namespace pots
{
    uint16_t results[MY_POTS_NUM];

    void init()
    {
        static_assert(manual_override_idx < MY_POTS_NUM);

        OPTanalogReference(EXTERNAL);
        OPTanalogChannel(adc_channels[0]);
    }

    void poll()
    {
        static uint8_t channel_idx = 0;

        results[channel_idx] = OPTanalogRead();
        if (++channel_idx >= MY_POTS_NUM) channel_idx = 0;
        OPTanalogChannel(adc_channels[channel_idx]);
    }

    uint16_t get_manual_override()
    {
        return results[manual_override_idx];
    }
} // namespace pots
