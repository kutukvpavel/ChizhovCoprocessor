#pragma once

#include "user.h"

namespace i2c
{
    void init();

    uint8_t get_drv_err();
    uint8_t get_drv_missing();

    void set_drv_missing_bit(uint8_t i, bool v);
    void set_drv_err_bit(uint8_t i, bool v);
    void set_manual_override(uint16_t v);
    void set_encoder_position(uint8_t i, unsigned long v);
} // namespace i2c
