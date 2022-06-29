/*
 * sacumi_melanjor_ATtiny2313.h
 *
 * Created: 25.06.2022 19:41:33
 *  Author: Alexey Lepeshkin
 */ 


#ifndef SACUMI_MELANJOR_ATTINY2313_H_
#define SACUMI_MELANJOR_ATTINY2313_H_

#include <stdbool.h>

void button_on_off_callback();
void motion_forward();
void motion_reverse();
void motion_off();
void button_on_off_single_click_callback();
void button_reset_single_click_callback();
void button_reset_long_press_click_callback();
void button_red_long_press_click_callback();
void button_red_long_release_click_callback();
void reset_led_on();
void reset_led_off();
void null_function();
void proccess_status(uint8_t button_t);

// config
#define buttons_debounce_time 15
#define buttons_interference_time 100
#define motion_reverse_blink_period 1000
#define motion_mix_blink_period 500
#define motion_mix_time 60000
#define motion_mix_period 3000 

// pins
// leds
#define pin_board_led PD0
#define pin_reset_led PB3 
#define pin_on_off_led PB2
// buttons
#define pin_button_on_off PB1
#define pin_button_reset PB0
#define pin_button_red PD6
// frequency converter
#define pin_motion_forward PD5 // pin for forward rotating
#define pin_motion_reverse PB4 // pin for reverse rotating

// variables
enum main_statuses {
	MAIN_STOP = 0,
	MOVING_FORWARD,
	MOVING_REVERSE,
	MOVING_MIX,
	RED_BUTTON_PRESSING,
	RED_BUTTON_RELEASED
};
uint8_t main_status = MAIN_STOP;
bool board_led_status = false;
uint32_t time_ms = 0;
uint32_t time_counter_board_led_blink = 0;
uint32_t time_counter_buttons = 0;
uint32_t time_counter_motion_reverse = 0;
uint32_t time_counter_motion_mix = 0;
enum motion_statuses {
	STOP = 0,
	FORWARD = 1,
	REVERSE = 2
};
uint8_t motion_status = STOP;
bool reset_led_status = false;
uint8_t motion_mix_period_counter = 0;
uint8_t motion_mix_time_counter = 0;
uint8_t motion_mix_period_var = motion_mix_period / motion_mix_blink_period;
uint8_t motion_mix_time_var = motion_mix_time / motion_mix_blink_period;
uint32_t test_timer1 = 0;
uint32_t test_timer2 = 0;


// enums
enum button_types {
	single_click_press = 0,
	single_click_release = 1,
	long_click = 2,

};
enum button_stages {
	passive = 0,
	debounce_press = 1,
	interference_press = 2,
	waiting_release = 3,
	waiting_release_longclick = 4,
	debounce_release = 5,
	interference_release = 6
};
enum button_name {
	button_on_off_single_click = 0,
	button_reset_single_click,
	button_reset_long_press_click,
	button_red_long_press_click,
	button_red_long_release_click
};


// structures
struct buttons 
{
	uint8_t pin;
	volatile uint8_t *port;
	uint8_t type;
	uint32_t time_counter;
	uint8_t stage;
	uint32_t time_for_long_click; // ms. how long press button until register long click
	void(*button_single_click_callback)(void);
	void(*button_long_click_press_callback)(void);
	void(*button_long_click_release_callback)(void);
	uint8_t past_stage;
	uint32_t last_time;
};

struct buttons button_on_off = {
	pin_button_on_off,
	&PINB,
	single_click_press,
	0,
	passive,
	0,
	button_on_off_single_click_callback,
	null_function,
	null_function
};

struct buttons button_reset = {
	pin_button_reset,
	&PINB,
	long_click,
	0,
	passive,
	3000,
	button_reset_single_click_callback,
	button_reset_long_press_click_callback,
	null_function
};

struct buttons button_red = {
	pin_button_red,
	&PIND,
	long_click,
	0,
	passive,
	0,
	null_function,
	button_red_long_press_click_callback,
	button_red_long_release_click_callback
};

// functions
void init_port();
void init_timers();
void button_tick(struct buttons *button);





#endif /* SACUMI_MELANJOR_ATTINY2313_H_ */