#pragma once

#include "user.h"

#define MY_ENCODERS_NUM 3

namespace encoders
{
    void init();

    unsigned long get_position(uint8_t i);
} // namespace encoders
