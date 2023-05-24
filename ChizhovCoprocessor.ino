#include <EEPROM.h>
#include <Wire.h>
#include <avr/wdt.h>

#include "src/OptPin.h"

#include "src/drv.h"
#include "src/pots.h"
#include "src/i2c_interface.h"
#include "src/encoders.h"
#include "src/my_conf.h"
#include "src/user.h"

#define MY_HEARTBEAT_PERIOD 1000 //ms

void setup()
{
    wdt_reset();
    delay(1);
    wdt_enable(WDTO_1S);

    //Init status LED
    pinMode(MY_LED_ARDUINO_PIN, OUTPUT);

    //Init I2C interface
    i2c::init();
    wdt_reset();
    //Init drive serial
    drv::init();
    wdt_reset();
    //Init encoders
    encoders::init();
    wdt_reset();
    //Init pots
    pots::init();
    wdt_reset();

    digitalWrite(MY_LED_ARDUINO_PIN, HIGH); //Open-collector-like
}

void loop()
{
    //Encoder and I2C processing run asynchronously (interrupt-based)
    //This loop has to execute the rest: drive polling (low-frequency task) and pot polling (high-freq task)
    //Last but not least: status LED

    static unsigned long last_led_toggle = 0;

    drv::poll();
    for (size_t i = 0; i < MY_DRIVES_NUM; i++)
    {
        i2c::set_drv_err_bit(i, drv::get_drv_err(i));
        i2c::set_drv_present_bit(i, drv::get_drv_present(i));
    }
    wdt_reset();

    pots::poll();
    i2c::set_manual_override(pots::get_manual_override());

    if ((millis() - last_led_toggle) > (MY_HEARTBEAT_PERIOD / 2))
    {
        MY_LED_PORT ^= MY_LED_MASK;
        last_led_toggle = millis();
    }

	wdt_reset();
}
