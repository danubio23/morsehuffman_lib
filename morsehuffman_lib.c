/*============================================================================*/
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
#define def_huffman_us_1 (def_cfg_huffman_us_0*2u)
#define def_huffman_us_idle def_cfg_huffman_us_0
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
#define def_symbol_blank 36u
#define def_symbol_invalid 37u
#define def_symbol_max 38u
#define def_fsm_state_idle 0u
#define def_fsm_state_processing 1u
#define def_fsm_state_print_dash 2u
#define def_fsm_state_print_dot 3u
#define def_ascii_val_0 48u
#define def_ascii_val_9 57u
#define def_ascii_val_cap_A 65u
#define def_ascii_val_cap_Z 90u
#define def_ascii_val_low_a 97u
#define def_ascii_val_low_z 122u

/*Prototypes*/
static void huffmanprint_func(void);
static void morsehuffman_pop(unsigned short * value, unsigned char * bits, op_modes mode);
static unsigned char index_translation(unsigned char par0);

/*Global variable declaration*/
typedef struct
{
  unsigned char value;
  unsigned char bits;
}huffmantable;

static char morsebuffer[def_cfg_buffer_size] = {0};
static char huffmanbuffer[def_cfg_buffer_size] = {0};
static unsigned int morseStrLenVar = 0;
static unsigned int huffmanStrLenVar = 0;

/*Local functions*/
static unsigned char index_translation(unsigned char par0)
{
    unsigned char index;

    if((par0 >= def_ascii_val_0) && (par0 <= def_ascii_val_9)) //0 - 9 ascii range
    {
        index = par0 - def_offset_digit;
    }
    else if((par0 >= def_ascii_val_cap_A) && (par0 <= def_ascii_val_cap_Z)) //A - Z ascii range
    {
        index = par0 - def_offset_capitalletter;
    }
    else if((par0 >= def_ascii_val_low_a) && (par0 <= def_ascii_val_low_z)) //a - z ascii range
    {
        index = par0 - def_offset_lowercaseletter;
    }
    else if(def_space_ascii == par0) //space
    {
        index = def_symbol_blank;
    }
    else
    {
       index = def_symbol_invalid;
    }

    return index;
}

static void morsehuffman_pop(unsigned short * value, unsigned char * bits, op_modes mode)
{
  char *ptrBuffer;
  unsigned char index;
  const huffmantable st_huffmantable[def_symbol_max] =
  {
    {0x20u,4}, //0
    {0x10u,4}, //1
    {0x50u,4}, //2
    {0x30u,4}, //3
    {0x70u,4}, //4
    {0x00u,5}, //5
    {0x08u,5}, //6
    {0xA0u,5}, //7
    {0x90u,5}, //8
    {0x88u,5}, //9
    {0x48u,5}, //A
    {0xE4u,6}, //B
    {0xD8u,5}, //C
    {0xF8u,5}, //D
    {0x60u,5}, //E
    {0xB4u,6}, //F
    {0xB0u,6}, //G
    {0xC4u,6}, //H
    {0xC8u,5}, //I
    {0xE0u,8}, //J
    {0xC2u,7}, //K
    {0xE8u,5}, //L
    {0x44u,6}, //M
    {0x68u,5}, //N
    {0xA8u,5}, //O
    {0x84u,6}, //P
    {0xE1u,8}, //Q
    {0xD0u,5}, //R
    {0xF0u,5}, //S
    {0x98u,5}, //T
    {0xB8u,5}, //U
    {0x40u,8}, //V
    {0x42u,7}, //W
    {0xE2u,7}, //X
    {0xC0u,7}, //Y
    {0x41u,8}, //Z
    {0x80u,6}, //Blank
    {0xFFu,0}  //Invalid
  };
  const unsigned short au16_symbols_morse[def_symbol_max] =
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

  if(enum_opmode_huffman == mode)
  {
    ptrBuffer = huffmanbuffer;
  }
  else
  {
    ptrBuffer = morsebuffer;
  }

  index = index_translation((unsigned char)ptrBuffer[0u]);

  if(enum_opmode_huffman == mode)
  {
        *value = (unsigned short)st_huffmantable[index].value;
        *bits = st_huffmantable[index].bits;
        huffmanStrLenVar--;
  }
  else
  {
        *value = au16_symbols_morse[index];
        morseStrLenVar--;
  }

  for(index = 0u; index < (def_cfg_buffer_size-1u); index++)
  {
      ptrBuffer[index] = ptrBuffer[index+1u];
  }
  ptrBuffer[def_cfg_buffer_size-1u] = (char)0u;
}

/*Public functions*/
void init_MorseHuffman(unsigned char led_id)
{
    hal_setuppin(led_id);
}

void morsehuffman_fsm(void)
{

    static unsigned char fsm_state = def_fsm_state_idle;
    static unsigned short chartobeprint = 0;
    static unsigned short duration_counter;

    switch(fsm_state)
    {
        case def_fsm_state_idle:
        {

            if(morseStrLenVar > 0u)
            {
               morsehuffman_pop(&chartobeprint,(unsigned char *)0u,enum_opmode_morse);
               if(def_val_invalid != chartobeprint)
               {
                fsm_state =  def_fsm_state_processing;
               }
            }
            else
            {
                huffmanprint_func();
            }

        }break;

        case def_fsm_state_processing:
        {
            if(morseStrLenVar == 0u)
            {
                fsm_state =  def_fsm_state_idle;
            }
            else
            {
                if((chartobeprint & def_val_dash) == def_val_dash)
                {
                    chartobeprint = chartobeprint << 3u;
                    duration_counter = 2;
                    fsm_state = def_fsm_state_print_dash;
                    huffmanprint_func();
                    hal_switch_led(def_led_state_on);
                }
                else if((chartobeprint & def_val_dot) == def_val_dot)
                {
                    chartobeprint = chartobeprint << 2u;
                    fsm_state = def_fsm_state_print_dot;
                    huffmanprint_func();
                    hal_switch_led(def_led_state_on);
                }
                else
                {
                    fsm_state = def_fsm_state_idle;
                }
            }
        }break;

        case def_fsm_state_print_dash:
        {
            if(duration_counter > 0u)
            {
                duration_counter--;
            }
            else
            {
                fsm_state = def_fsm_state_processing;
                huffmanprint_func();
                hal_switch_led(def_led_state_off);
            }
        }break;
        case def_fsm_state_print_dot:
        {
            fsm_state = def_fsm_state_processing;
            huffmanprint_func();
            hal_switch_led(def_led_state_off);
        }break;
        default:
        {
            fsm_state = def_fsm_state_idle;
        }break;
    }
}

static void huffmanprint_func(void)
{
  unsigned char bits;
  unsigned short huffmancode;
  unsigned char index;
  unsigned char toPrint = 0u;

  if(0u != huffmanStrLenVar)
  {
    morsehuffman_pop(&huffmancode,&bits,enum_opmode_huffman);

    hal_switch_led(def_led_state_off);

    for(index = 0u; index < bits; index++)
    {
        toPrint = (unsigned char)((huffmancode >> (8u-(1u+index))) & 0x01u);
        hal_switch_led(def_led_state_on);
        if(1u == toPrint)
        {
            hal_delayexecution_us((unsigned char)def_huffman_us_1);
        }
        else
        {
            hal_delayexecution_us((unsigned char)def_cfg_huffman_us_0);
        }
        hal_switch_led(def_led_state_off);
		hal_delayexecution_us((unsigned char)def_huffman_us_idle);
    }
  }
}

void morsehuffman_msg(const char *str, op_modes mode)
{
   unsigned char index;
   unsigned char lengthval = 0;
   
   while(str[lengthval] != 0)
   {
	  lengthval++; 
   }

   if(enum_opmode_huffman == mode)
   {
		if((def_cfg_buffer_size - huffmanStrLenVar) < lengthval)
		{
			lengthval = def_cfg_buffer_size - huffmanStrLenVar;
		}
		
        if((str != (const char *)0u) && (lengthval > 0u))
        {
            for(index = 0u; index < lengthval; index++)
            {
                huffmanbuffer[huffmanStrLenVar+index] = str[index];
            }
            huffmanStrLenVar += lengthval;
        }
   }
   else
   {
		if((def_cfg_buffer_size - morseStrLenVar - def_offset_space_char ) < lengthval)
		{
			lengthval = def_cfg_buffer_size - morseStrLenVar - def_offset_space_char;
		}
		
        if((str != (const char *)0u) && (lengthval > 0u))
        {
            for(index = 0u; index < (lengthval+def_offset_space_char); index++)
            {
                if(index < lengthval)
                {
                    morsebuffer[morseStrLenVar+index] = str[index];
                }
                else
                {
                    morsebuffer[morseStrLenVar+index] = (char)def_space_ascii;
                }
            }
            morseStrLenVar += (unsigned int)(lengthval+def_offset_space_char);
        }
   }
}