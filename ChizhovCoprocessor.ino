#include <EEPROM.h>
#include <Wire.h>
#include <avr/wdt.h>

#include "OptPin.h"

#include "drv.h"
#include "pots.h"
#include "i2c_interface.h"
#include "encoders.h"
#include "my_conf.h"

#define MY_HEARTBEAT_PERIOD 1000 //ms

void setup()
{
    //Init status LED
    pinMode(MY_LED_ARDUINO_PIN, OUTPUT);

    //Init I2C interface
    i2c::init();
    //Init drive serial
    drv::init();
    //Init encoders
    encoders::init();
    //Init pots
    pots::init();

    digitalWrite(MY_LED_ARDUINO_PIN, HIGH); //Open-collector-like
}

void loop()
{
    //Encoder and I2C processing run asynchronously (interrupt-based)
    //This loop has to execute the rest: drive polling (low-frequency task) and pot polling (high-freq task)
    //Last but not least: status LED
    static uint16_t led_counter = 0;

    drv::poll();
    for (size_t i = 0; i < MY_DRIVES_NUM; i++)
    {
        i2c::set_drv_err_bit(i, drv::get_drv_err(i));
        i2c::set_drv_present_bit(i, drv::get_drv_present(i));
    }

    pots::poll();
    i2c::set_manual_override(pots::get_manual_override());

    if (led_counter++ > (MY_HEARTBEAT_PERIOD / 2))
    {
        MY_LED_PORT ^= MY_LED_MASK;
        led_counter = 0;
    }

	wdt_reset();
}
