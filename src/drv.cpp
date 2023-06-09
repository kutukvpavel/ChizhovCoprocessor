#include "drv.h"

#include <Arduino.h>
#include "OptPin.h"
#include "my_conf.h"

#define MY_DRV_BUF_LEN 16_ui8
#define MY_DRV_RESPONSE_TIMEOUT 5_ui8 //ms
#define MY_STATUS_RSP_LEN 3_ui8
#define MY_LOAD_ERR_RSP_LEN 4_ui8

namespace drv
{
    enum drv_cmds : uint8_t
    {
        read_shaft_status = 0,
        read_shaft_error,

        CMDS_NUM
    };

    bool err[MY_DRIVES_NUM];
    bool present[MY_DRIVES_NUM];
    int16_t load[MY_DRIVES_NUM];

    void discard_in_buffer()
    {
        while (Serial.read() > 0) delay(1);
    }

    void init()
    {
        static_assert(MY_DRIVES_NUM < 0b111);
        const char dbg_greeting[] = "Goodnight moon!\n";

        for (auto &&i : err)
        {
            i = false;
        }
        for (auto &&i : present)
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
        discard_in_buffer();
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
    const char* get_drv_command(drv_cmds c, uint8_t station_idx)
    {
        const uint8_t command_bytes[drv_cmds::CMDS_NUM] = {
            0x3E,
            0x39
        };
        static uint8_t buffer[MY_DRV_BUF_LEN];

        uint8_t len = 0;
        buffer[len++] = station_addrs[station_idx];
        buffer[len++] = command_bytes[c];
        buffer[len] = calc_chk(buffer, len);
        buffer[++len] = '\0';
        return reinterpret_cast<const char*>(buffer);
    }
    bool parse_shaft_status_response(uint8_t drv_idx)
    {
        uint8_t sum = 0;
        bool res = false;
        for (uint8_t i = 0; i < (MY_STATUS_RSP_LEN - 1); i++)
        {
            uint8_t current = static_cast<uint8_t>(Serial.read());
            sum += current;
            if (i == 0)
            {
                if (current != station_addrs[drv_idx]) return true;
            }
            else if (i == 1)
            {
                res = (current == 0x00);
            }
        }
        if (sum != static_cast<uint8_t>(Serial.read())) return true;
        return res;
    }
    int16_t parse_shaft_error_response(uint8_t drv_idx)
    {
        static const uint16_t error_code = 0xFFFF;

        uint8_t sum = 0;
        uint16_t res;
        for (uint8_t i = 0; i < (MY_LOAD_ERR_RSP_LEN - 1); i++)
        {
            uint8_t current = static_cast<uint8_t>(Serial.read());
            sum += current;
            if (i == 0)
            {
                if (current != station_addrs[drv_idx]) return error_code;
            }
            else if (i == 1)
            {
                res = static_cast<uint16_t>(current) << 8;
            }
            else if (i == 2)
            {
                res |= current;
            }
        }
        if (sum != static_cast<uint8_t>(Serial.read())) return error_code;
        return static_cast<int16_t>(res);
    }

    void poll()
    {
        const char dbg_heartbeat[] = ">\n";
        const uint8_t rsp_length[drv_cmds::CMDS_NUM] = {
            MY_STATUS_RSP_LEN,
            MY_LOAD_ERR_RSP_LEN
        };
        static uint8_t drv_idx = 0;
        static bool read_phase = false;
        static bool load_phase = false; //There are 2 major phases: polling motor shaft load error and polling status

        if (drv_idx >= MY_DRIVES_NUM) //Debug header
        {
            if (read_phase)
            {
                discard_in_buffer();
            }
            else
            {
                set_addr(0);
                Serial.write(dbg_heartbeat);
            }
        }
        else
        {
            if (read_phase)
            {
                uint8_t i = MY_DRV_RESPONSE_TIMEOUT;
                while ((Serial.available() < rsp_length[load_phase ? drv_cmds::read_shaft_error : drv_cmds::read_shaft_status])
                    && (i > 0))
                {
                    delay(1);
                    --i;
                };
                bool p = i > 0;
                present[drv_idx] = p;
                if (p) {
                    if (load_phase)
                    {
                        load[drv_idx] = parse_shaft_error_response(drv_idx);
                    }
                    else
                    {
                        err[drv_idx] = parse_shaft_status_response(drv_idx);
                    }
                }
                else err[drv_idx] = false;
                discard_in_buffer();
            }
            else
            {
                set_addr(drv_idx + 1_ui8);
                delayMicroseconds(1);
                Serial.write(get_drv_command(load_phase ? drv_cmds::read_shaft_error : drv_cmds::read_shaft_status, drv_idx));
            }
        }
        if (read_phase)
        {
            if (++drv_idx >= (MY_DRIVES_NUM + 1_ui8)) 
            {
                drv_idx = 0;
                load_phase = !load_phase;
            }
            read_phase = false;
        }
        else
        {
            read_phase = true;
            Serial.flush(); //Blocks until transmission is over
        }
    }

    bool get_drv_err(uint8_t i)
    {
        return err[i];
    }
    bool get_drv_present(uint8_t i)
    {
        return present[i];
    }
    uint8_t get_load(uint8_t i)
    {
        uint16_t res = static_cast<uint16_t>(abs(load[i])) >> 4; // divide by 16, should result in ~0.1 deg resolution
        if (res > 0xFF) res = 0xFF;
        return static_cast<uint8_t>(res);
    }
} // namespace drv
