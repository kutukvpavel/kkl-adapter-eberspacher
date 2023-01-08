#ifndef __AVR_ATtiny13A__ //IntelliSense definitions
    #define __AVR_ATtiny13A__
#endif
#ifndef F_CPU
    #define F_CPU 1200000ULL //9.6 MHz internal clock with CKDIV8 enabled
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/fuse.h>

#ifndef _BV
    #define _BV(v) (1u << (v))
#endif

#define DEBUG 0

#define LOOP_DELAY_US 1000
#define WAIT_DELAY_MS 1600 //1.6S
#define RESET_TIMEOUT_CYCLES 8000 //To wait before resetting transition counter
#define WAIT_TIMER_INCREMENT_MS 400 //0.4S for each /RTS pulse
#define OUTPUT_PIN PB0
#define RTS_INPUT_PIN PB1
#define DBG_MAIN_OUTPUT_PIN PB3
#define DBG_TIMER_OUTPUT_PIN PB4

FUSES = 
{
    .low = LFUSE_DEFAULT,
    .high = HFUSE_DEFAULT
};

static volatile uint16_t assertion_timer = 0; //Count output pin assertion time, max ~30sec
static volatile uint16_t wait_timer = 0; //1.6S wait time (from first /RTS edge to the assertion edge)
static volatile uint8_t transition_cnt = 0;
static uint8_t last_rts;

ISR(TIM0_COMPA_vect)
{
    if (wait_timer > 0)
    {
        --wait_timer;
    }
    else if (assertion_timer > 0)
    {
        PORTB |= _BV(OUTPUT_PIN);
        if (--assertion_timer == 0) transition_cnt = 0;
    }
    else
    {
        PORTB &= ~_BV(OUTPUT_PIN);
    }
#if DEBUG
    PORTB ^= _BV(DBG_TIMER_OUTPUT_PIN);
#endif
}

void setup(void)
{
    DDRB |= _BV(OUTPUT_PIN)
#if DEBUG
     | _BV(DBG_MAIN_OUTPUT_PIN) | _BV(DBG_TIMER_OUTPUT_PIN)
#endif
    ;
    TCCR0A |= _BV(WGM01);            // set timer counter mode to CTC
    TCCR0B |= _BV(CS01); // set prescaler to 8 (CLK=1200000Hz/8/150=1kHz, 1mS)
    OCR0A = 150;                     // set Timer's counter max value
    TIMSK0 |= _BV(OCIE0A);           // enable Timer CTC interrupt
    sei(); // enable global interrupts

    last_rts = PINB & _BV(RTS_INPUT_PIN); //Monitor /RTS value and detect transistions
}

void loop(void)
{
    static uint16_t reset_cnt = 0;

    uint8_t current_rts = PINB & _BV(RTS_INPUT_PIN); //Monitor /RTS value and detect transistions
    if (current_rts != last_rts)
    {
        last_rts = current_rts;
        reset_cnt = 0;

        cli();
        ++transition_cnt;
        sei();
        switch (transition_cnt)
        {
        case 1:
            cli();
            wait_timer = WAIT_DELAY_MS;
            sei();
            break;
        case 2:
            assertion_timer = 2400; // 2.4 S
            break;
        case 4:
            cli();
            wait_timer += WAIT_TIMER_INCREMENT_MS;
            sei();
            assertion_timer = 1900; // 1.9 S
            break;
        case 6:
            cli();
            wait_timer += WAIT_TIMER_INCREMENT_MS;
            sei();
            assertion_timer = 3000; // 3.0 S
            break;
        default:
            assertion_timer = 0;
            break;
        }
    }
    if (reset_cnt++ > RESET_TIMEOUT_CYCLES || transition_cnt > 6)
    {
        transition_cnt = 0;
        reset_cnt = 0;
        cli();
        wait_timer = 0;
        assertion_timer = 0;
        sei();
    }
}

int main(void)
{
    setup();
    while (1)
    {
        loop();
#if DEBUG
        PORTB ^= _BV(DBG_MAIN_OUTPUT_PIN);
#endif
        _delay_us(LOOP_DELAY_US);
    }
}