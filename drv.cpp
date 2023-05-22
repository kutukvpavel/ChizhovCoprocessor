#include "drv.h"

#include <Arduino.h>
#include "OptPin.h"
#include "my_conf.h"

#define MY_DRV_BUF_LEN 16

namespace drv
{
    enum drv_cmds : uint8_t
    {
        read_shaft_status = 0,

        CMDS_NUM
    };

    bool err[MY_DRIVES_NUM];

    void init()
    {
        static_assert(MY_DRIVES_NUM < 0b111);
        const char dbg_greeting[] = "Goodnight moon!\n";

        for (auto &&i : err)
        {
            i = false;
        }

        //Default addr (000): redirects serial to debug header
        pin_t::SetDirection(&pin_addr_a, pin_t::Output);
        pin_t::Write(&pin_addr_a, false);
        pin_t::SetDirection(&pin_addr_b, pin_t::Output);
        pin_t::Write(&pin_addr_b, false);
        pin_t::SetDirection(&pin_addr_c, pin_t::Output);
        pin_t::Write(&pin_addr_c, false);

        Serial.begin(9600);
        while (Serial.read() > 0) delay(1);
        Serial.write(dbg_greeting);
    }

    void set_addr(uint8_t i)
    {
        pin_t::Write(&pin_addr_a, i & _BV(0));
        pin_t::Write(&pin_addr_b, i & _BV(1));
        pin_t::Write(&pin_addr_c, i & _BV(2));
    }
    uint8_t calc_chk(const uint8_t* buf, uint8_t len)
    {
        uint8_t sum = 0;

        for (size_t i = 0; i < len; i++)
        {
            sum += buf[i];
        }
        return sum;
    }
    const uint8_t* get_drv_command(drv_cmds c, uint8_t station_idx)
    {
        const uint8_t command_bytes[drv_cmds::CMDS_NUM] = {
            0x3E
        };
        static uint8_t buffer[MY_DRV_BUF_LEN];

        uint8_t len = 0;
        buffer[len++] = station_addrs[station_idx];
        buffer[len++] = command_bytes[c];
        buffer[len] = calc_chk(buffer, len);
        buffer[++len] = '\0';
        return buffer;
    }

    void poll()
    {
        const char dbg_heartbeat[] = ">\n";
        static uint8_t drv_idx = 0;
        static bool read_phase = false;

        if (drv_idx == 0)
        {
            if (read_phase)
            {
                while (Serial.read() > 0) delay(1);
            }
            else
            {
                Serial.write(dbg_heartbeat);
            }
        }
        else
        {
            if (read_phase)
            {

            }
            else
            {
                set_addr(i);
                delayMicroseconds(1);
                Serial.write(get_drv_command(drv_cmds::read_shaft_status, i));
            }
        }
        if (read_phase)
        {
            if (++drv_idx >= MY_DRIVES_NUM) drv_idx = 0;
            read_phase = false;
        }
        else
        {
            read_phase = true;
            Serial.flush(); //Blocks until transmission is over
        }
    }

    uint8_t get_drv_err()
    {
        uint8_t buffer = 0;
        for (size_t i = 0; i < MY_DRIVES_NUM; i++)
        {
            if (err[i]) buffer |= _BV(i);
        }
        return buffer;
    }
} // namespace drv
