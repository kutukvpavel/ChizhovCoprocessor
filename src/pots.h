#pragma once

#include "user.h"

namespace pots
{
    void init();

    void poll();

    uint16_t get_manual_override();
} // namespace pots

