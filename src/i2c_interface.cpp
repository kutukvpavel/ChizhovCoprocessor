#include "i2c_interface.h"

#include <Arduino.h>
#include <Wire.h>
#include "my_conf.h"

#include "util/crc16.h"

#define COPROCESSOR_INIT_BYTE 0xA0

namespace i2c
{
    struct memory_map_t
    {
        uint16_t encoder_pos[MY_ENCODERS_NUM];
        uint16_t manual_override;
        uint8_t encoder_btn_pressed;
        uint8_t drv_error_bitfield;
        uint8_t drv_missing_bitfield;
        uint8_t drv_load[MY_DRIVES_NUM];
        uint8_t reserved1;
        uint8_t crc;
    };
    static volatile memory_map_t map = {
        .encoder_pos = { 0, 0, 0 },
        .manual_override = 1000,
        .encoder_btn_pressed = 0,
        .drv_error_bitfield = 0,
        .drv_missing_bitfield = 0,
        .drv_load = { 0, 0, 0 }
    };
    static volatile bool connected = false;

    void request_response() //This runs in an interrupt context
    {
        static uint8_t buffer[sizeof(map)];
        
        memcpy(buffer, &map, sizeof(buffer) - 1); //Excluding last CRC byte
        map.crc = 0xFF;
        for (uint8_t i = 0; i < (sizeof(buffer) - 1); i++)
        {
            map.crc = _crc8_ccitt_update(map.crc, buffer[i]);
        }
        buffer[sizeof(buffer) - 1] = map.crc;
        Wire.write(buffer, sizeof(buffer));
        map.encoder_btn_pressed = 0;
    }
    void receive(int data)
    {
        uint8_t b = static_cast<uint8_t>(Wire.read() & 0xFF);
        connected = (b == COPROCESSOR_INIT_BYTE);
    }

    void init()
    {
        static_assert((sizeof(map.drv_error_bitfield) * __CHAR_BIT__) >= MY_DRIVES_NUM);
        static_assert(sizeof(map) % 2 == 0); //16-bit word alignment required for receivend end DMA

        Wire.begin(MY_I2C_ADDR);
        Wire.onRequest(request_response);
        Wire.onReceive(receive);
    }
    bool get_connected()
    {
        return connected;
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
            map.drv_missing_bitfield |= BV8(i);
            sei();
        }
        else
        {
            cli();
            map.drv_missing_bitfield &= ~BV8(i);
            sei();
        }
    }
    void set_drv_err_bit(uint8_t i, bool v)
    {
        if (v)
        {
            cli();
            map.drv_error_bitfield |= BV8(i);
            sei();
        }
        else
        {
            cli();
            map.drv_error_bitfield &= ~BV8(i);
            sei();
        }
    }
    void set_manual_override(uint16_t v)
    {
        cli();
        map.manual_override = v;
        sei();
    }
    void set_encoder_position(uint8_t i, uint16_t v)
    {
        cli();
        map.encoder_pos[i] = v;
        sei();
    }
    void set_encoder_button(uint8_t i, bool v)
    {
        static uint8_t buttons_mask = 0xFF; //Inverted

        cli();
        //Mask out buttons already read as pressed, until let go
        uint8_t mask = BV8(i);
        if (v) 
        {
            map.encoder_btn_pressed |= mask & buttons_mask;
            buttons_mask &= ~mask;
        }
        else
        {
            //map.encoder_btn_pressed &= ~mask; I2C ISR handler clears this register, this ensures no missed presses
            buttons_mask |= mask;
        }
        sei();
    }
    void set_load(uint8_t i, uint8_t v)
    {
        cli();
        map.drv_load[i] = v;
        sei();
    }
} // namespace i2c
