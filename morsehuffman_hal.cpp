/*
 *  Original code by Daniel Rubio at Feb 2023
 *
 *  This is free software. You can redistribute it and/or modify.
 *
 *  Author mail: daniel.rubio23@gmail.com
 */

#include "morsehuffman_hal.h"

char outputled = 13;

/*Abstraction layer*/
void hal_switch_led(unsigned char state)
{
  digitalWrite(outputled, state);
}

void hal_delayexecution_us(unsigned char us) 
{
	delayMicroseconds(us);
}

void hal_setuppin(unsigned char led_id) 
{
	outputled = led_id;
    pinMode(led_id, OUTPUT);
}