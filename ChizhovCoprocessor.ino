#include <EEPROM.h>
#include <Wire.h>
#include <avr/wdt.h>

#include "OptPin.h"

#include "drv.h"
#include "pots.h"
#include "i2c_interface.h"
#include "encoders.h"
#include "my_conf.h"

void setup()
{
    //Init status LED
    pinMode(MY_LED_PIN, OUTPUT);

    //Init I2C interface
    i2c::init();
    //Init drive serial
    drv::init();
    //Init encoders
    encoders::init();
    //Init pots
    pots::init();

    digitalWrite(MY_LED_PIN, HIGH); //Open-collector-like
}

void loop()
{
    //Encoder and I2C processing run asynchronously (interrupt-based)
    //This loop has to execute the rest: drive polling (low-frequency task) and pot polling (high-freq task)
    //Last but not least: status LED

    drv::poll();
    pots::poll();
    i2c::set_manual_override(pots::get_manual_override());

	wdt_reset();
}
