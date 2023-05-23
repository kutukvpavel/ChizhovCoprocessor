#include "pots.h"

#include "my_conf.h"
#include "OptPin.h"
#include "average/average.h"

namespace pots
{
    average* results[MY_POTS_NUM];

    void init()
    {
        static_assert(manual_override_idx < MY_POTS_NUM);

        for (size_t i = 0; i < MY_POTS_NUM; i++)
        {
            results[i] = new average(averaging_len);
        }

        OPTanalogReference(EXTERNAL);
        OPTanalogChannel(adc_channels[0]);
    }

    void poll()
    {
        static uint8_t channel_idx = 0;

        results[channel_idx]->enqueue(OPTanalogRead());
        if (++channel_idx >= MY_POTS_NUM) channel_idx = 0;
        OPTanalogChannel(adc_channels[channel_idx]);
    }

    uint16_t get_manual_override()
    {
        return static_cast<uint16_t>(results[manual_override_idx]->get_average());
    }
} // namespace pots
