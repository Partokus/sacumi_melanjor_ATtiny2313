/*
 * sacumi_melanjor_ATtiny2313.c
 *
 * Created: 25.06.2022 19:38:29
 * Author : Alexey Lepeshkin
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "sacumi_melanjor_ATtiny2313.h"

int main(void)
{
	init_port();
	init_timers();
	sei();
	
    while (1) 
    {
		if (time_board_led < time_ms)
		{
			time_board_led = time_ms + 1000;
			
			board_led_status = !board_led_status;
			if (board_led_status) PORTD |= 1 << pin_board_led;
			else PORTD &= ~(1 << pin_board_led);
		}
    }
}

void init_port()
{
	// board led pin
	DDRD = 1 << pin_board_led;
	PORTD = 1 << pin_board_led;
	board_led_status = true;
}

void init_timers()
{
	TCCR0A = 1 << WGM01; // compare mode
	TCCR0B = 1 << CS01 | 1 << CS00; // 64 prescaler
	OCR0A = 62; // 1 ms period
	TIMSK = 1 << OCIE0A; // timer compare interrupt enable
	TIFR = 1 << OCF0A;
}

ISR(TIMER0_COMPA_vect)
{
	time_ms++;
}
