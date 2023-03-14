/*
 *  Original code by Daniel Rubio at December 2022
 *
 *  This is free software. You can redistribute it and/or modify.
 *
 *  Author mail: daniel.rubio23@gmail.com
 */

#ifndef morsehuffman_hal_h_
#define morsehuffman_hal_h_

#include "Arduino.h"

void hal_switch_led(unsigned char state);
void hal_delayexecution_us(unsigned char us); 
void hal_setuppin(unsigned char led_id);

#endif