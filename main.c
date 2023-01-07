#ifndef __AVR_ATtiny13A__ //IntelliSense definitions
    #define __AVR_ATtiny13A__
#endif
#ifndef F_CPU
    #define F_CPU 1200000ULL //9.6 MHz internal clock
#endif

#include <avr/io.h>
#include <util/delay.h>

#ifndef _BV
    #define _BV(v) (1u << (v))
#endif

#define LOOP_DELAY_US 1000
#define RESET_TIMEOUT_CYCLES 20000 //To wait before resetting transition counter
#define ASSERTION_TIME_INCREMENT_MS 1000
#define OUTPUT_PIN PB0
#define RTS_INPUT_PIN PB1

void setup(void)
{
    DDRB |= _BV(OUTPUT_PIN);
}

void loop(void)
{
    static uint8_t last_rts = 0; //Monitor /RTS value and detect transistions
    static uint16_t reset_cnt = 0;
    static uint8_t transition_cnt = 0;
    static uint16_t assertion_timer = 0; //Count output pin assertion time, max ~30sec

    uint8_t current_rts = PINB & _BV(RTS_INPUT_PIN);
    if (current_rts != last_rts)
    {
        last_rts = current_rts;
        reset_cnt = 0;

        if (++transition_cnt % 2 == 0) //Got a whole pulse
        {
            assertion_timer += ASSERTION_TIME_INCREMENT_MS;
        }
    }
    else if (reset_cnt++ > RESET_TIMEOUT_CYCLES)
    {
        transition_cnt = 0;
        reset_cnt = 0;
    }

    if (assertion_timer > 0)
    {
        if (--assertion_timer == 0)
        {
            PORTB &= ~_BV(OUTPUT_PIN);
        }
        else    
        {
            PORTB |= _BV(OUTPUT_PIN);
        }
    }
}

int main(void)
{
    setup();
    while (1)
    {
        loop();
        _delay_us(LOOP_DELAY_US);
    }
}