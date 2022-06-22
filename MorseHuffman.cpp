/*
 *  Original code by Daniel Rubio at December 2022
 *
 *  This is free software. You can redistribute it and/or modify.
 *
 *  Author mail: daniel.rubio23@gmail.com
 */

#include "MorseHuffman.h"

/*constants*/
#define def_val_dot  0x8000u
#define def_val_dash 0xC000u
#define def_val_end  0x0000u
#define def_val_blank 0x0001u
#define def_val_invalid 0xFFFFu
#define def_offset_space_char 1u
#define def_space_ascii 32u
#define def_led_state_off 0u
#define def_led_state_on 1u

/*config*/
#define buffer_size 16u
#define numchar_timeout 4u

struct huffmantable
{
  unsigned short value;
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
  {0x0006u,4}, //0
  {0x0005u,4}, //1
  {0x0003u,4}, //2
  {0x0002u,4}, //3
  {0x0001u,4}, //4
  {0x001Fu,5}, //5
  {0x001Eu,5}, //6
  {0x001Du,5}, //7
  {0x001Bu,5}, //8
  {0x001Au,5}, //9
  {0x0016u,5}, //A
  {0x0070u,7}, //B
  {0x0029u,6}, //C
  {0x0009u,5}, //D
  {0x0019u,5}, //E
  {0x0011u,6}, //F
  {0x0030u,6}, //G
  {0x0012u,5}, //H
  {0x0013u,5}, //I
  {0x0141u,9}, //J
  {0x0051u,7}, //K
  {0x0001u,5}, //L
  {0x0020u,6}, //M
  {0x0011u,5}, //N
  {0x0015u,5}, //O
  {0x0071u,7}, //P
  {0x0020u,7}, //Q
  {0x000Eu,5}, //R
  {0x000Fu,5}, //S
  {0x0017u,5}, //T
  {0x0000u,5}, //U
  {0x0021u,7}, //V
  {0x0031u,6}, //W
  {0x00A1u,8}, //X
  {0x0021u,6}, //Y
  {0x0140u,9}, //Z
  {0x0039u,6}, //Blank
  {0xFFFFu,0}  //Invalid
};

const unsigned short au16_symbols[enum_symbol_max] =
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

static char morsebuffer[buffer_size] = {0};
static char huffmanbuffer[buffer_size] = {0};
static unsigned int morseStrLenVar = 0;
static unsigned int huffmanStrLenVar = 0;
static unsigned char outputled = 13u;

static void huffmanprint_func(void);
static void morsehuffman_pop(unsigned short * value, unsigned char * bits, op_modes mode);
static void switch_led(unsigned char state);
static unsigned char index_translation(unsigned char par0);

/*Static interfaces*/
static void switch_led(unsigned char state)
{
  digitalWrite(outputled, state);
}

static unsigned char index_translation(unsigned char par0)
{
    unsigned char index = 0;
    
    if(par0 >= 48 && par0 <= 57) //0 - 9 ascii range
    {
        index = par0 - 48u;   
    }
    else if(par0 >= 65 && par0 <= 90) //A - Z ascii range
    {
        index = par0 - 55u;
    }
    else if(par0 >= 97 && par0 <= 122) //a - z ascii range
    {
        index = par0 - 87u;
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
  unsigned int *ptrStrLen;
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
    *value = au16_symbols[index];
    *bits = 0;
  }

  for(index = 0; index < buffer_size-1; index++)
  {
      if(0 != ptrBuffer[index+1])
      {
          ptrBuffer[index] = ptrBuffer[index+1];
      }
      else
      {
          ptrBuffer[index] = 0;
          break;
      }        
  }  
}

/*Public API*/
void init_MorseHuffman(unsigned char led_id)
{
    outputled = led_id;
    pinMode(led_id, OUTPUT);
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
               else
               {
                morseStrLenVar--;
               }
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
                    switch_led(def_led_state_on);
                }
                else if((chartobeprint & def_val_dot) == def_val_dot)
                {
                    chartobeprint = chartobeprint << 2;
                    fsm_state = enum_fsm_state_print_dot;
                    huffmanprint_func();
                    switch_led(def_led_state_on);
                }
                else if(((chartobeprint & def_val_blank) == def_val_blank) || ((chartobeprint & def_val_end) == def_val_end))
                {
                  morseStrLenVar--;
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
                switch_led(def_led_state_off);
            }
        }break;
        case enum_fsm_state_print_dot:
        { 
            fsm_state = enum_fsm_state_processing;
            huffmanprint_func();
            switch_led(def_led_state_off);
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
  unsigned int toPrintAux = 0;
  unsigned int toPrint = 0;
  unsigned char totalBits = 0;

  if(0 != huffmanStrLenVar)
  {      
    while(0 != huffmanStrLenVar)
    {
      if(16u >= (totalBits + 9u))
      {
        morsehuffman_pop(&huffmancode,&bits,enum_opmode_huffman);
        totalBits += bits;
        toPrintAux = huffmancode << (16u - totalBits);
        toPrint += toPrintAux;
      }
      else
      {
        break;
      }
    }

      switch_led(def_led_state_off);
      switch_led(def_led_state_on);
      switch_led(def_led_state_off);
      switch_led(def_led_state_on);

      for(index = 0; index < totalBits; index++)
      {
        switch_led((toPrint >> 16u-(1+index)) & 0x0001);
      }

      switch_led(def_led_state_on);
      switch_led(def_led_state_off);
      switch_led(def_led_state_on);
      switch_led(def_led_state_off);
  }
}

void morsehuffman_msg(char *str, unsigned int lengthval, op_modes mode)
{
   unsigned int *ptrStrLen;
   unsigned char *ptrBuffer;
   unsigned char index;
   unsigned int actuallength = lengthval + def_offset_space_char;

   if(enum_opmode_huffman == mode)
   {
    ptrStrLen = &huffmanStrLenVar;
    ptrBuffer = huffmanbuffer;
   }
   else
   {
    ptrStrLen = &morseStrLenVar;
    ptrBuffer = morsebuffer;
   }
   
   if(str != 0 && lengthval > 0)
   {    
        if(*ptrStrLen>0)
        {
          ptrBuffer[*ptrStrLen++] = def_space_ascii;
        }
        
        for(index = 0; index < lengthval; index++)
        {
            if((*ptrStrLen+index) < buffer_size)
            {
                ptrBuffer[*ptrStrLen+index] = str[index]; 
            }
            else
            {
                *ptrStrLen = buffer_size;
                ptrBuffer[*ptrStrLen] = def_space_ascii;
                break;
            }
        }
        *ptrStrLen += lengthval;
   }
}