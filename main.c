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
		if (time_counter_board_led_blink < time_ms)
		{
			time_counter_board_led_blink = time_ms + 1000;
			
			//board_led_status = !board_led_status;
			//if (board_led_status) PORTD |= 1 << pin_board_led;
			//else PORTD &= ~(1 << pin_board_led);
		}
		
		if (time_counter_buttons < time_ms)
		{
			time_counter_buttons = time_ms + 1;
			
			button_tick(&button_on_off);
			button_tick(&button_reset);
			button_tick(&button_red);
		}
		
		
    }
}

void init_port()
{
	// board led pin
	DDRD = 1 << pin_board_led;
	PORTD = 1 << pin_board_led;
	board_led_status = true;
	
	// buttons
	// on/off button
	DDRB = 0;
	PORTB = 1 << pin_button_on_off;
	// reset button
	PORTB = 1 << pin_button_reset;
	// red button
	PORTD |= 1 << pin_button_red;
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

// process button
void button_tick(struct buttons *button)
{
	bool pin_level = ((1 << button->pin) & *button->port);
	uint32_t past_time;
	
	if (button->stage != passive)
	{
		past_time = time_ms - button->time_counter; 
	}
	
	switch (button->stage)
	{
		case passive:
			if (pin_level == false) 
			{
				button->stage = debounce_press;
				button->time_counter = time_ms;
			}
		break;
		
		case debounce_press:
			if (past_time >= buttons_debounce_time) 
			{
				button->stage = interference;
			}
		break;
		
		case interference:
			if (pin_level == true) // if we catch interference
			{
				button->stage = passive; // stop proccess button
			}
			
			if (past_time >= (buttons_interference_time + buttons_debounce_time))
			{
				button->stage = waiting_release;
				
				if (button->type == single_click_press) button->button_callback();
			}
		break;
		
		case waiting_release:
			if (pin_level == true)
			{
				if (button->type == long_click) button->button_callback(); // if button was released before long click called
				
				button->stage = debounce_release;
				button->time_counter = time_ms;
			} else if (button->type == long_click) {
				if (past_time >= button->time_for_long_click) // if time for long click became
				{
					button->stage = waiting_release_longclick;
					button->button_callback();
				}
			}
		break;
		
		case waiting_release_longclick:
			if (pin_level == true)
			{	
				button->stage = debounce_release;
				button->time_counter = time_ms;
			}
		break;
		
		case debounce_release:
			if (past_time >= buttons_debounce_time)
			{
				button->stage = passive;
			}
		break;
	}
}

void button_on_off_callback()
{
	board_led_status = !board_led_status;
	if (board_led_status) PORTD |= 1 << pin_board_led;
	else PORTD &= ~(1 << pin_board_led);
}