#pragma once

#include "user.h"

namespace drv
{
    void init();

    void poll();

    bool get_drv_err(uint8_t i);
    bool get_drv_present(uint8_t i);
    uint8_t get_load(uint8_t i);
} // namespace drv
