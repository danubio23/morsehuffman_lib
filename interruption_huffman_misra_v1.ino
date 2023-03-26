/*============================================================================*/
/*
 *  Original code by Daniel Rubio at Feb 2023
 *
 *  This is free software. You can redistribute it and/or modify.
 *
 *  Author mail: daniel.rubio23@gmail.com
 */
 
/*Configurable parameters*/
#define DEF_INTERRUPTION_PIN 12u //botón en GPIO12 (D6)
#define DEF_DELAYDURATION 10u
#define DEF_MAXDURATION_0_U 13u
/*END configurable parameters*/

/*Definitions*/
#define DEF_MAXDURATIONPULSE_U DEF_MAXDURATION_0_U*2u
#define DEF_VALUEVAR_BITS 8u
#define DEF_HUFFMANTABLESZ 37u

/*Prototypes*/
//void ICACHE_RAM_ATTR handleInterrupt(void);
void handleInterrupt(void);

static void hal_delay(unsigned int delayduration);
static void hal_functionToPrint(char character);
static unsigned long hal_functionToGetMicros(void);
static void hal_setup(unsigned char input_pin);

static unsigned char replaceBit(unsigned char n, unsigned char m, unsigned char i);
static char binary_search(void);

/*Global variable declaration*/
typedef struct
{
  unsigned char value;
  char character;
}huffmantable;

const static huffmantable st_huffmantable[DEF_HUFFMANTABLESZ] =
{
  {0x00u, ' '},
  {0x08u, 'U'},
  {0x10u, '4'},
  {0x20u, '3'},
  {0x30u, '2'},
  {0x40u, 'Q'},
  {0x41u, 'J'},
  {0x42u, 'V'},
  {0x44u, 'F'},
  {0x48u, 'C'},
  {0x50u, '1'},
  {0x60u, 'L'},
  {0x68u, 'S'},
  {0x70u, '0'},
  {0x80u, 'B'},
  {0x84u, 'G'},
  {0x88u, 'N'},
  {0x90u, 'T'},
  {0x98u, 'O'},
  {0xA0u, 'I'},
  {0xA8u, 'R'},
  {0xB0u, 'H'},
  {0xB4u, 'M'},
  {0xB8u, 'A'},
  {0xC0u, 'K'},
  {0xC2u, 'W'},
  {0xC4u, 'P'},
  {0xC8u, 'E'},
  {0xD0u, '9'},
  {0xD8u, '8'},
  {0xE0u, 'Z'},
  {0xE1u, 'X'},
  {0xE2u, 'Y'},
  {0xE4u, 'D'},
  {0xE8u, '7'},
  {0xF0u, '6'},
  {0xF8u, '5'},
};

static unsigned long currentMicros = 0u;
static unsigned long prev_currentMicros = 0u;
static unsigned char idx = 0u;
static unsigned char value = 0u;

/*Abstraction layer*/
static void hal_delay(unsigned int delayduration)
{
  delay(delayduration);
}

static void hal_functionToPrint(char character)
{
  Serial.print(character);
}

static unsigned long hal_functionToGetMicros(void)
{
  return micros();
}

static void hal_setup(unsigned char input_pin)
{
  Serial.begin(9600);   // initialize serial communication at 9600 BPS
  pinMode(input_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(input_pin),handleInterrupt,CHANGE);
}

/*Regular functions*/
void handleInterrupt(void) {
  static unsigned char state = 0u;
  static unsigned long duration;

  if(0u == state)
  {
      state = 1u;
  }
  else
  {
      state = 0u;
  }

  currentMicros = hal_functionToGetMicros();
  duration = currentMicros - prev_currentMicros;
  if((duration < (DEF_MAXDURATIONPULSE_U)) && (idx < DEF_VALUEVAR_BITS) && (0u == state))
  {
    if(duration < DEF_MAXDURATION_0_U)
    {
      value = replaceBit(value,0u,(DEF_VALUEVAR_BITS-1u)-idx);
      idx++;
    }
    else
    {
      value = replaceBit(value,1u,(DEF_VALUEVAR_BITS-1u)-idx);
      idx++;
    }
  }
  prev_currentMicros = currentMicros;
}

static char binary_search(void)
{
    unsigned char left = 0u;
    unsigned char right = DEF_HUFFMANTABLESZ - 1u;
    char outcome = (char)-1;

    while (left <= right) {
        unsigned char mid = (left + right) / (unsigned char)2u;
        if (st_huffmantable[mid].value == value) {
            outcome = (char)mid;
        }
        else if (st_huffmantable[mid].value < value) {
            left = mid + 1u;
        }
        else {
            right = mid - 1u;
        }
    }

    return outcome;
}

static unsigned char replaceBit(unsigned char n, unsigned char m, unsigned char i)
{
    unsigned char mask = (unsigned char)((unsigned char)1u << (unsigned char)i);
    return (n & ~mask) | (m << i);
}



/*classic program structure*/
void setup(void) {
  hal_setup(DEF_INTERRUPTION_PIN);
  currentMicros = hal_functionToGetMicros();
  prev_currentMicros = hal_functionToGetMicros();
}

void loop() {

    char outcome;
    if(idx > 0u)
    {
        outcome = binary_search();

        if(((outcome > (char)0u) && (outcome<(char)DEF_HUFFMANTABLESZ)) || (((char)0u == outcome) && (5u == idx)))
        {

          hal_functionToPrint(st_huffmantable[(unsigned char)outcome].character);
          value = 0u;
          idx = 0u;
        }
        else
        {
          if(idx == DEF_VALUEVAR_BITS)
          {
            value = 0u;
            idx = 0u;
          }
        }
    }

  hal_delay(DEF_DELAYDURATION);
}

