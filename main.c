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
		if (time_counter_buttons < time_ms)
		{
			time_counter_buttons = time_ms + 1;
			
			button_tick(&button_on_off);
			button_tick(&button_reset);
			button_tick(&button_red);
		}
		
		if (main_status == MOVING_REVERSE && time_counter_motion_reverse < time_ms)
		{
			time_counter_motion_reverse = time_ms + motion_reverse_blink_period;
			
			if (reset_led_status) reset_led_off();
			else reset_led_on();
		}
		
		if (main_status == MOVING_MIX)
		{
			if (time_counter_motion_mix_time < time_ms) // if time for mix rotation end up
			{
				proccess_status(button_on_off_single_click); // stop mix rotation and go to status MOVING_FORWARD
			} else { 
				if (time_counter_motion_mix_blink < time_ms && ((time_ms - time_counter_motion_mix_time) > motion_mix_blink_hysteresis)) // blink led
				{
					time_counter_motion_mix_blink = time_ms + motion_mix_blink_period;
					
					if (reset_led_status) reset_led_off();
					else reset_led_on();
				}
				
				if (time_counter_motion_mix_direction < time_ms  && ((time_ms - time_counter_motion_mix_time) > motion_mix_period_hysteresis)) // change direction
				{
					time_counter_motion_mix_direction = time_ms + motion_mix_period;
					
					if (motion_status == FORWARD) motion_reverse();
					else motion_forward();
				}
			}
		}
    }
}

void init_port()
{
	// leds
	// board led pin
	DDRD = 1 << pin_board_led;
	PORTD = 1 << pin_board_led;
	board_led_status = true;
	// reset led pin
	DDRB = 1 << pin_reset_led;
	// pin on/off led
	DDRB |= 1 << pin_on_off_led;
	
	// buttons
	// on/off button
	PORTB |= 1 << pin_button_on_off;
	// reset button
	PORTB |= 1 << pin_button_reset;
	// red button
	PORTD |= 1 << pin_button_red;
	
	// frequency converter
	// motion forward pin
	DDRD |= 1 << pin_motion_forward;
	// motion reverse pin
	DDRB |= 1 << pin_motion_reverse;
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
				button->stage = interference_press;
			}
		break;
		
		case interference_press:
			if (pin_level == true) // if we catch interference
			{
				button->stage = passive; // stop proccess button
				return;
			}
			
			if (past_time >= (buttons_interference_time + buttons_debounce_time))
			{
				button->stage = waiting_release;
				
				if (button->type == single_click_press) button->button_single_click_callback();
			}
		break;
		
		case waiting_release:
			if (pin_level == true)
			{	
				button->stage = debounce_release;
				button->past_stage = waiting_release;
				button->last_time = button->time_counter;
				button->time_counter = time_ms;
			} else if (button->type == long_click) {
				if (past_time >= button->time_for_long_click) // if time for long click became
				{
					button->stage = waiting_release_longclick;
					button->past_stage = waiting_release;
					button->button_long_click_press_callback();
				}
			}
		break;
		
		case waiting_release_longclick:
			if (pin_level == true)
			{	
				button->stage = debounce_release;
				button->past_stage = waiting_release_longclick;
				button->last_time = button->time_counter;
				button->time_counter = time_ms;
			}
		break;
		
		case debounce_release:
			if (past_time >= buttons_debounce_time)
			{
				button->stage = interference_release;
			}
		break;
		
		case interference_release:
			if (pin_level == false) // if we catch interference
			{
				button->time_counter = button->last_time;
				button->stage = button->past_stage;
				return;
			}
		
			if (past_time >= (buttons_interference_time + buttons_debounce_time))
			{
				button->stage = passive;
				if (button->type == long_click && button->past_stage == waiting_release) button->button_single_click_callback(); // if button was released before long click called
				if (button->type == long_click && button->past_stage == waiting_release_longclick) button->button_long_click_release_callback();
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

// moving forward
void motion_forward()
{
	motion_status = FORWARD;
	PORTD |= 1 << pin_motion_forward;
	PORTB &= ~(1 << pin_motion_reverse);
	// turn on on/off led
	PORTB |= 1 << pin_on_off_led;
}

// moving reverse
void motion_reverse()
{
	motion_status = REVERSE;
	PORTB |= 1 << pin_motion_reverse;
	PORTD &= ~(1 << pin_motion_forward);
}

// don't moving
void motion_off()
{
	motion_status = STOP;
	PORTD &= ~(1 << pin_motion_forward);
	PORTB &= ~(1 << pin_motion_reverse);
	// turn off on/off led
	PORTB &= ~(1 << pin_on_off_led);
}

// status proccessing
void proccess_status(uint8_t button_t)
{
	switch (main_status)
	{
		case MAIN_STOP:
		if (button_t == button_on_off_single_click)
		{
			main_status = MOVING_FORWARD;
			motion_forward();
		} else if (button_t == button_reset_single_click)
		{
			
		} else if (button_t == button_reset_long_press_click)
		{
			
		}
		break;
		
		case MOVING_FORWARD:
		if (button_t == button_on_off_single_click)
		{
			main_status = MAIN_STOP;
			motion_off();
			reset_led_off();
		} else if (button_t == button_reset_single_click)
		{
			main_status = MOVING_REVERSE;
			motion_reverse();
			time_counter_motion_reverse = time_ms + motion_reverse_blink_period;
			reset_led_on();
		} else if (button_t == button_reset_long_press_click)
		{
			main_status = MOVING_MIX;
			motion_reverse();
			// start timers
			time_counter_motion_mix_blink = time_ms + motion_mix_blink_period;
			time_counter_motion_mix_direction = time_ms + motion_mix_period;
			time_counter_motion_mix_time = time_ms + motion_mix_time;
			reset_led_on();
		}
		break;
		
		case MOVING_REVERSE:
		if (button_t == button_on_off_single_click)
		{
			main_status = MAIN_STOP;
			motion_off();
			reset_led_off();
		} else if (button_t == button_reset_single_click)
		{
			main_status = MOVING_FORWARD;
			motion_forward();
			reset_led_off();
		} else if (button_t == button_reset_long_press_click)
		{
			main_status = MOVING_MIX;
			motion_forward();
			// start timers
			time_counter_motion_mix_blink = time_ms + motion_mix_blink_period;
			time_counter_motion_mix_direction = time_ms + motion_mix_period;
			time_counter_motion_mix_time = time_ms + motion_mix_time;
		}
		break;
		
		case MOVING_MIX:
		if (button_t == button_on_off_single_click)
		{
			main_status = MOVING_FORWARD;
			motion_forward();
			reset_led_off();
		} else if (button_t == button_reset_single_click)
		{
			main_status = MOVING_FORWARD;
			motion_forward();
			reset_led_off();
		} else if (button_t == button_reset_long_press_click)
		{
			// restart timer
			time_counter_motion_mix_time = time_ms + motion_mix_time;
		}
		break;
		
		case RED_BUTTON_PRESSING:

		break;
		
		case RED_BUTTON_RELEASED:
		if (button_t == button_reset_single_click)
		{
			main_status = MAIN_STOP;
			reset_led_off();
		}
		break;	
	}
	
	if (button_t == button_red_long_press_click)
	{
		main_status = RED_BUTTON_PRESSING;
		motion_off();
		reset_led_on();
	} else if (button_t == button_red_long_release_click)
	{
		main_status = RED_BUTTON_RELEASED;
	}
}

void button_on_off_single_click_callback()
{
	proccess_status(button_on_off_single_click);
}

void button_reset_single_click_callback()
{
	proccess_status(button_reset_single_click);
}

void button_reset_long_press_click_callback()
{
	proccess_status(button_reset_long_press_click);
}

void button_red_long_press_click_callback()
{
	proccess_status(button_red_long_press_click);
}

void button_red_long_release_click_callback()
{
	proccess_status(button_red_long_release_click);
}

void reset_led_on()
{
	PORTB |= 1 << pin_reset_led;
	reset_led_status = true;
}

void reset_led_off()
{
	PORTB &= ~(1 << pin_reset_led);
	reset_led_status = false;
}

void null_function()
{
	
}