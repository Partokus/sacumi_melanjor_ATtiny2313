/*
 * sacumi_melanjor_ATtiny2313.h
 *
 * Created: 25.06.2022 19:41:33
 *  Author: Alexey Lepeshkin
 */ 


#ifndef SACUMI_MELANJOR_ATTINY2313_H_
#define SACUMI_MELANJOR_ATTINY2313_H_

#include <stdbool.h>

// functions
void init_port();
void init_timers();

// pins
#define pin_board_led PD0

// variables
bool board_led_status = false;
uint32_t time_ms = 0;
uint32_t time_board_led = 0;






#endif /* SACUMI_MELANJOR_ATTINY2313_H_ */