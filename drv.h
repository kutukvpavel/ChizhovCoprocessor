#pragma once

#include <stdint.h>
#include <stddef.h>

namespace drv
{
    void init();

    void poll();

    bool get_drv_err(uint8_t i);
    bool get_drv_present(uint8_t i);
} // namespace drv
