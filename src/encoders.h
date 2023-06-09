#pragma once

#include "user.h"

#define MY_ENCODERS_NUM 3

namespace encoders
{
    void init();

    uint16_t get_position(uint8_t i);
    bool get_button_pressed(uint8_t i);
    void poll_buttons();
} // namespace encoders
