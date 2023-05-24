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
        uint8_t drv_missing_bitfield;
    };
    static memory_map_t map = {
        .encoder_pos = { 0, 0, 0 },
        .manual_override = 1000,
        .drv_error_bitfield = 0,
        .drv_missing_bitfield = 0
    };

    void request_response() //This runs in an interrupt context
    {
        static uint8_t buffer[sizeof(map)];
        
        memcpy(buffer, &map, sizeof(buffer));
        Wire.write(buffer, sizeof(buffer));
    }

    void init()
    {
        static_assert((sizeof(map.drv_error_bitfield) * __CHAR_BIT__) >= MY_DRIVES_NUM);

        Wire.begin(MY_I2C_ADDR);
        Wire.onRequest(request_response);
    }

    uint8_t get_drv_err()
    {
        return map.drv_error_bitfield;
    }
    uint8_t get_drv_missing()
    {
        return map.drv_missing_bitfield;
    }

    void set_drv_missing_bit(uint8_t i, bool v)
    {
        if (v)
        {
            cli();
            map.drv_missing_bitfield |= _BV(i);
            sei();
        }
        else
        {
            cli();
            map.drv_missing_bitfield &= ~_BV(i);
            sei();
        }
    }
    void set_drv_err_bit(uint8_t i, bool v)
    {
        if (v)
        {
            cli();
            map.drv_error_bitfield |= _BV(i);
            sei();
        }
        else
        {
            cli();
            map.drv_error_bitfield &= ~_BV(i);
            sei();
        }
    }
    void set_manual_override(uint16_t v)
    {
        cli();
        map.manual_override = v;
        sei();
    }
    void set_encoder_position(uint8_t i, long v)
    {
        cli();
        map.encoder_pos[i] = v;
        sei();
    }
} // namespace i2c
