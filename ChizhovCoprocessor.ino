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

enum class led_status : uint8_t
{
    OK = 0,
    drive_missing,
    drive_error,
    init,

    LEN
};

void setup()
{
    wdt_reset();
    delay(1);
    wdt_enable(WDTO_1S);

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
    wdt_reset();

    digitalWrite(MY_LED_ARDUINO_PIN, HIGH); //Open-collector-like
}

void loop()
{
    //Encoder and I2C processing run asynchronously (interrupt-based)
    //This loop has to execute the rest: drive polling (low-frequency task) and pot polling (high-freq task)
    //Last but not least: status LED
    static led_status status = led_status::OK;

    drv::poll();
    for (size_t i = 0; i < MY_DRIVES_NUM; i++)
    {
        static_assert(MY_DRIVES_NUM == MY_ENCODERS_NUM);

        i2c::set_drv_err_bit(i, drv::get_drv_err(i));
        i2c::set_drv_missing_bit(i, !drv::get_drv_present(i));
        i2c::set_encoder_position(i, encoders::get_position(i));
        i2c::set_load(i, drv::get_load(i));
    }
    status = led_status::OK;
    if (i2c::get_drv_missing() > 0) status = led_status::drive_missing;
    if (i2c::get_drv_err() > 0) status = led_status::drive_error;
    if (!i2c::get_connected()) status = led_status::init;

    pots::poll();
    i2c::set_manual_override(pots::get_manual_override());
    encoders::poll_buttons();
    for (size_t i = 0; i < MY_ENCODERS_NUM; i++)
    {
        i2c::set_encoder_button(i, encoders::get_button_pressed(i));
    }

    supervize_led(status);
	wdt_reset();
}

void supervize_led(led_status s)
{
    static const uint16_t period_short[static_cast<uint8_t>(led_status::LEN)] = { 500, 1300, 200, 200 };
    static const uint16_t period_long[static_cast<uint8_t>(led_status::LEN)] = { 500, 1300, 200, 1500 };
    static const uint16_t* const period_db[] = { period_short, period_long };
    static unsigned long last_led_toggle = 0;
    static uint8_t period_type = 0;

    static_assert(arraySize(period_db) == 2);

    if ((millis() - last_led_toggle) > period_db[period_type][static_cast<uint8_t>(s)])
    {
        last_led_toggle = millis();
        MY_LED_PORT ^= MY_LED_MASK;
        period_type ^= 1_ui8;
    }
}