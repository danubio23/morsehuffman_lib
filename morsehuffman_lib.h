/*
 *  Original code by Daniel Rubio at December 2022
 *
 *  This is free software. You can redistribute it and/or modify.
 *
 *  Author mail: daniel.rubio23@gmail.com
 */

#ifndef morsehuffman_lib_h_
#define morsehuffman_lib_h_

typedef enum
{
    enum_opmode_morse,
    enum_opmode_huffman
}enum_op_modes;

typedef enum_op_modes op_modes;

void init_MorseHuffman(unsigned char led_id);
void morsehuffman_fsm(void);
void morsehuffman_msg(char *str, unsigned int lengthval, op_modes mode);

#endif