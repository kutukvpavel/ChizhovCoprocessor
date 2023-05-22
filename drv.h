#pragma once

namespace drv
{
    void init();

    void poll();

    bool get_drv_err(uint8_t i);
} // namespace drv
