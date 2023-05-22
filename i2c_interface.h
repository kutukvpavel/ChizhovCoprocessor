#pragma once

namespace i2c
{
    void init();

    void set_drv_err_bit(uint8_t i, bool v);
    void set_manual_override(uint16_t v);
} // namespace i2c
