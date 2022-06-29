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

// config
#define buttons_debounce_time 10
#define buttons_interference_time 100

// pins
#define pin_board_led PD0
//buttons
#define pin_button_on_off PB1
#define pin_button_reset PB0
#define pin_button_red PD6

// variables
bool board_led_status = false;
uint32_t time_ms = 0;
uint32_t time_counter_board_led_blink = 0;
uint32_t time_counter_buttons = 0;


// enums
enum button_types {
	single_click_press = 0,
	single_click_release = 1,
	long_click = 2
};
enum button_stages {
	passive = 0,
	debounce_press = 1,
	interference = 2,
	waiting_release = 3,
	waiting_release_longclick = 4,
	debounce_release = 5
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
	void(*button_callback)(void);
};

struct buttons button_on_off = {
	pin_button_on_off,
	&PINB,
	single_click_press,
	0,
	passive,
	0,
	button_on_off_callback
};

struct buttons button_reset = {
	pin_button_reset,
	&PINB,
	long_click,
	0,
	passive,
	3000,
	button_on_off_callback
};

struct buttons button_red = {
	pin_button_red,
	&PIND,
	long_click,
	0,
	passive,
	3000,
	button_on_off_callback
};

// functions
void init_port();
void init_timers();
void button_tick(struct buttons *button);





#endif /* SACUMI_MELANJOR_ATTINY2313_H_ */