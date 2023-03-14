/*
 *  Original code by Daniel Rubio at Feb 2023
 *
 *  This is free software. You can redistribute it and/or modify.
 *
 *  Author mail: daniel.rubio23@gmail.com
 */

#include "morsehuffman_lib.h"
#include "morsehuffman_hal.h"
#include "morsehuffman_cfg.h"

/*END configurable parameters*/

/*Definitions*/
#define def_huffman_us_1 def_cfg_huffman_us_0*3
#define def_val_dot  0x8000u
#define def_val_dash 0xC000u
#define def_val_end  0x0000u
#define def_val_blank 0x0001u
#define def_val_invalid 0xFFFFu
#define def_offset_space_char 1u
#define def_space_ascii 32u
#define def_led_state_off 0u
#define def_led_state_on 1u
#define def_offset_digit 48u
#define def_offset_capitalletter 55u
#define def_offset_lowercaseletter 87u

/*Prototypes*/
static void hal_switch_led(unsigned char state);
static void hal_delayexecution_us(unsigned char us);
static void hal_setuppin(unsigned char led_id);

static void huffmanprint_func(void);
static void morsehuffman_pop(unsigned short * value, unsigned char * bits, op_modes mode);
static unsigned char index_translation(unsigned char par0);

/*Global variable declaration*/
struct huffmantable
{
  unsigned char value;
  unsigned char bits;
};

typedef enum
{
    enum_fsm_state_idle,
    enum_fsm_state_processing,
    enum_fsm_state_print_dash,
    enum_fsm_state_print_dot,
    enum_fsm_state_print_space                   
}enum_fsm_states;

typedef enum
{
    enum_symbol_0,
    enum_symbol_1,
    enum_symbol_2,
    enum_symbol_3,
    enum_symbol_4,
    enum_symbol_5,
    enum_symbol_6,
    enum_symbol_7,
    enum_symbol_8,
    enum_symbol_9,
    enum_symbol_A,
    enum_symbol_B, 
    enum_symbol_C,
    enum_symbol_D,
    enum_symbol_E,
    enum_symbol_F,
    enum_symbol_G,
    enum_symbol_H,
    enum_symbol_I,
    enum_symbol_J,
    enum_symbol_K,
    enum_symbol_L,
    enum_symbol_M,
    enum_symbol_N,
    enum_symbol_O,
    enum_symbol_P,
    enum_symbol_Q,
    enum_symbol_R,
    enum_symbol_S,
    enum_symbol_T,
    enum_symbol_U,
    enum_symbol_V,
    enum_symbol_W,
    enum_symbol_X,
    enum_symbol_Y,
    enum_symbol_Z,
    enum_symbol_blank,
    enum_symbol_invalid,
    enum_symbol_max
}enum_symbols;

const huffmantable st_huffmantable[enum_symbol_max] = 
{
  {0x70u,4}, //0
  {0x50u,4}, //1
  {0x30u,4}, //2
  {0x20u,4}, //3
  {0x10u,4}, //4
  {0xF8u,5}, //5
  {0xF0u,5}, //6
  {0xE8u,5}, //7
  {0xD8u,5}, //8
  {0xD0u,5}, //9
  {0xB8u,5}, //A
  {0x80u,6}, //B
  {0x48u,5}, //C
  {0xE4u,6}, //D
  {0xC8u,5}, //E
  {0x44u,6}, //F
  {0x84u,6}, //G
  {0xB0u,6}, //H
  {0xA0u,5}, //I
  {0x41u,8}, //J
  {0xC0u,7}, //K
  {0x60u,5}, //L
  {0xB4u,6}, //M
  {0x88u,5}, //N
  {0x98u,5}, //O
  {0xC4u,6}, //P
  {0x40u,8}, //Q
  {0xA8u,5}, //R
  {0x68u,5}, //S
  {0x90u,5}, //T
  {0x08u,5}, //U
  {0x42u,7}, //V
  {0xC2u,7}, //W
  {0xE1u,8}, //X
  {0xE2u,7}, //Y
  {0xE0u,8}, //Z
  {0x00u,5}, //Blank
  {0xFFu,0}  //Invalid
};

const unsigned short au16_symbols_morse[enum_symbol_max] =
{
    0xDB6Cu,//0
    0xB6D8u,//1
    0xADB0u,//2
    0xAB60u,//3
    0xAAC0u,//4
    0xAA80u,//5
    0xD540u,//6
    0xDAA0u,//7
    0xDB50u,//8
    0xDB68u,//9
    0xB000u,//Aa
    0xD500u,//Bb
    0xD680u,//Cc
    0xD400u,//Dd
    0x8000u,//Ee
    0xAD00u,//Ff
    0xDA00u,//Gg
    0xAA00u,//Hh
    0xA000u,//Ii
    0xB6C0u,//Jj
    0xD600u,//Kk
    0xB500u,//Ll
    0xD800u,//Mm
    0xD000u,//Nn
    0xDB00u,//Oo
    0xB680u,//Pp
    0xDAC0u,//Qq
    0xB400u,//Rr
    0xA800u,//Ss
    0xC000u,//Tt
    0xAC00u,//Uu
    0xAB00u,//Vv
    0xB600u,//Ww
    0xD580u,//Xx
    0xD6C0u,//Yy
    0xDA80u,//Zz
    0x0001u,//Blank
    def_val_invalid //Invalid
};

unsigned char morsebuffer[def_cfg_buffer_size] = {0};
unsigned char huffmanbuffer[def_cfg_buffer_size] = {0};
unsigned int morseStrLenVar = 0;
unsigned int huffmanStrLenVar = 0;

/*Local functions*/
static unsigned char index_translation(unsigned char par0)
{
    unsigned char index = 0;
    
    if(par0 >= '0' && par0 <= '9') //0 - 9 ascii range
    {
        index = par0 - def_offset_digit;   
    }
    else if(par0 >= 'A' && par0 <= 'Z') //A - Z ascii range
    {
        index = par0 - def_offset_capitalletter;
    }
    else if(par0 >= 'a' && par0 <= 'z') //a - z ascii range
    {
        index = par0 - def_offset_lowercaseletter;
    }
    else if(def_space_ascii == par0) //space
    {
        index = enum_symbol_blank;
    }
    else
    {
       index = enum_symbol_invalid;
    }
    
    return index;
}

static void morsehuffman_pop(unsigned short * value, unsigned char * bits, op_modes mode)
{
  unsigned char *ptrBuffer;
  unsigned char index;

  if(enum_opmode_huffman == mode)
  {
    ptrBuffer = huffmanbuffer;
  }
  else
  {
    ptrBuffer = morsebuffer;
  }

  index = index_translation(ptrBuffer[0]);

  if(enum_opmode_huffman == mode)
  {
        *value = st_huffmantable[index].value;
        *bits = st_huffmantable[index].bits;
        huffmanStrLenVar--;
  }
  else
  {
		*value = au16_symbols_morse[index];
		morseStrLenVar--;
  }
  
  for(index = 0; index < def_cfg_buffer_size-1; index++)
  {
	  ptrBuffer[index] = ptrBuffer[index+1];   
  }  
  ptrBuffer[def_cfg_buffer_size-1] = 0;
}

/*Public functions*/
void init_MorseHuffman(unsigned char led_id)
{
	hal_setuppin(led_id);
}

void morsehuffman_fsm(void)
{
	
    static char fsm_state = enum_fsm_state_idle;
    static unsigned short chartobeprint;
    static unsigned short duration_counter;
	
    switch(fsm_state)
    {
        case enum_fsm_state_idle:
        {
			
            if(morseStrLenVar > 0)
            {
               morsehuffman_pop(&chartobeprint,0,enum_opmode_morse);
               if(def_val_invalid != chartobeprint)
               {
                fsm_state =  enum_fsm_state_processing;
               }
            }
			else
			{
				huffmanprint_func();
			}
				
        }break;

        case enum_fsm_state_processing:
        {
            if(morseStrLenVar == 0)
            {
                fsm_state =  enum_fsm_state_idle;
            }
            else
            {
                if((chartobeprint & def_val_dash) == def_val_dash)
                {
                    chartobeprint = chartobeprint << 3u;
                    duration_counter = 2;
                    fsm_state = enum_fsm_state_print_dash;
                    huffmanprint_func();
                    hal_switch_led(def_led_state_on);
                }
                else if((chartobeprint & def_val_dot) == def_val_dot)
                {
                    chartobeprint = chartobeprint << 2;
                    fsm_state = enum_fsm_state_print_dot;
                    huffmanprint_func();
                    hal_switch_led(def_led_state_on);
                }
                else if(((chartobeprint & def_val_blank) == def_val_blank) || ((chartobeprint & def_val_end) == def_val_end))
                {
                  fsm_state = enum_fsm_state_idle;  
                }
                else
                {
                    /*do nothing*/
                }
            }
        }break;

        case enum_fsm_state_print_dash:
        { 
            if(duration_counter > 0)
            {
                duration_counter--;
            }
            else
            {
                fsm_state = enum_fsm_state_processing;
                huffmanprint_func();
                hal_switch_led(def_led_state_off);
            }
        }break;
        case enum_fsm_state_print_dot:
        { 
            fsm_state = enum_fsm_state_processing;
            huffmanprint_func();
            hal_switch_led(def_led_state_off);
        }break;
        default:
        {
            
        }break;
    }
}

static void huffmanprint_func(void)
{
  unsigned char bits;
  unsigned short huffmancode;
  unsigned char index;
  unsigned char toPrint = 0;

  if(0 != huffmanStrLenVar)
  {      
    morsehuffman_pop(&huffmancode,&bits,enum_opmode_huffman);

	hal_switch_led(def_led_state_off);
	
    for(index = 0; index < bits; index++)
    {
		toPrint = (huffmancode >> 8u-(1+index)) & 0x01;
		hal_switch_led(def_led_state_on);
		if(1 == toPrint)
		{
			hal_delayexecution_us(def_huffman_us_1);
		}
		else
		{
			hal_delayexecution_us(def_cfg_huffman_us_0);		
		}
		hal_switch_led(def_led_state_off);	
    }
  }
}

void morsehuffman_msg(char *str, unsigned int lengthval, op_modes mode)
{
   unsigned char index;
   
   if(enum_opmode_huffman == mode)
   {
		if(str != 0 && lengthval > 0 && (lengthval+huffmanStrLenVar) < def_cfg_buffer_size)
		{
			for(index = 0; index < lengthval; index++)
			{
				if(index < lengthval)
				{
					huffmanbuffer[huffmanStrLenVar+index] = str[index]; 
				}
			}
			huffmanStrLenVar += lengthval;
		}		
   }
   else
   {
	    if(str != 0 && lengthval > 0 && (lengthval+def_offset_space_char+morseStrLenVar) < def_cfg_buffer_size)
		{
			for(index = 0; index < lengthval+def_offset_space_char; index++)
			{
				if(index < lengthval)
				{
					morsebuffer[morseStrLenVar+index] = str[index]; 
				}
				else
				{
					morsebuffer[morseStrLenVar+index] = def_space_ascii;
				}
			}
			morseStrLenVar += lengthval+def_offset_space_char;
		}				
   }
}
