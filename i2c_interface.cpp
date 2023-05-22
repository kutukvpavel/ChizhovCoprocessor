#include "i2c_interface.h"

#include <Arduino.h>
#include <Wire.h>
#include "my_conf.h"

namespace i2c
{
    struct memory_map_t
    {
        long encoder_pos[MY_ENCODERS_NUM];
        uint16_t manual_override;
        uint8_t drv_error_bitfield;
    };
    static memory_map_t map = {
        .encoder_pos = { 0, 0, 0 },
        .manual_override = 1000,
        .drv_error_bitfield = 0
    };

    void request_response()
    {
        static uint8_t buffer[sizeof(map)];
        
        cli();
        memcpy(buffer, &map, sizeof(buffer));
        sei();

        Wire.write(buffer, sizeof(buffer));
    }

    void init()
    {
        static_assert((sizeof(map.drv_error_bitfield) * __CHAR_BIT__) >= MY_DRIVES_NUM);

        Wire.begin(MY_I2C_ADDR);
        Wire.onRequest(request_response);
    }

    void set_drv_err_bit(uint8_t i, bool v)
    {
        if (v)
        {
            map.drv_error_bitfield |= _BV(i);
        }
        else
        {
            map.drv_error_bitfield &= ~_BV(i);
        }
    }
    void set_manual_override(uint16_t v)
    {
        map.manual_override = v;
    }
} // namespace i2c
