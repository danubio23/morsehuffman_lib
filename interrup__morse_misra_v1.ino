/*============================================================================*/
/*
 *  Original code by Daniel Rubio at Feb 2023
 *
 *  This is free software. You can redistribute it and/or modify.
 *
 *  Author mail: daniel.rubio23@gmail.com
 */

/*Configurable parameters*/
#define DEF_INTERRUPTION_PIN 12u //Interruption using GPIO12 (D6)
#define DEF_DELAYDURATION 200u
#define DEF_TIME_UNIT_MS 500u
/*END configurable parameters*/

/*Definitions*/
#define DEF_MS_BETWEEN_CHARS (DEF_TIME_UNIT_MS*3u)-100u
#define DEF_MS_BETWEEN_WORDS (DEF_TIME_UNIT_MS*5u)-100u
#define DEF_MS_DOT DEF_TIME_UNIT_MS+200u
#define DEF_MS_MAX_WAIT_LOW (DEF_MS_BETWEEN_CHARS)/DEF_DELAYDURATION

#define DEF_VALUEVAR_BITS 16u
#define DEF_MORSETABLESZ 36u
#define DEF_STATE_LOW  0u
#define DEF_STATE_HIGH 1u

/*Prototypes*/
//void ICACHE_RAM_ATTR handleInterrupt(void);
void handleInterrupt(void);
static void hal_delay(unsigned int delayduration);
static void hal_functionToPrint(char character);
static unsigned long hal_functionToGetMillis(void);
static void hal_setup(unsigned char input_pin);

static unsigned short replaceBit(unsigned short n, unsigned short m, unsigned short i);
static char binary_search(void);
static void printachar(void);

/*Global variable declaration*/
typedef struct
{
  unsigned short value;
  char character;
}morsetable;

const static morsetable st_morsetable[DEF_MORSETABLESZ] =
{
  {0x8000u, 'E'},
  {0xA000u, 'I'},
  {0xA800u, 'S'},
  {0xAA00u, 'H'},
  {0xAA80u, '5'},
  {0xAAC0u, '4'},
  {0xAB00u, 'V'},
  {0xAB60u, '3'},
  {0xAC00u, 'U'},
  {0xAD00u, 'F'},
  {0xADB0u, '2'},
  {0xB000u, 'A'},
  {0xB400u, 'R'},
  {0xB500u, 'L'},
  {0xB600u, 'W'},
  {0xB680u, 'P'},
  {0xB6C0u, 'J'},
  {0xB6D8u, '1'},
  {0xC000u, 'T'},
  {0xD000u, 'N'},
  {0xD400u, 'D'},
  {0xD500u, 'B'},
  {0xD540u, '6'},
  {0xD580u, 'X'},
  {0xD600u, 'K'},
  {0xD680u, 'C'},
  {0xD6C0u, 'Y'},
  {0xD800u, 'M'},
  {0xDA00u, 'G'},
  {0xDA80u, 'Z'},
  {0xDAA0u, '7'},
  {0xDAC0u, 'Q'},
  {0xDB00u, 'O'},
  {0xDB50u, '8'},
  {0xDB68u, '9'},
  {0xDB6Cu, '0'}
};

static unsigned long currentMillis = 0u;
static unsigned long prev_currentMillis = 0u;
static unsigned short value = 0u;
static unsigned char idx = 0u;


/*Abstraction layer*/
static void hal_delay(unsigned int delayduration)
{
  delay(delayduration);
}

static void hal_functionToPrint(char character)
{
  Serial.print(character);
}

static unsigned long hal_functionToGetMillis(void)
{
  return millis();
}

static void hal_setup(unsigned char input_pin)
{
  Serial.begin(9600);   // initialize serial communication at 9600 BPS
  pinMode(input_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(input_pin),handleInterrupt,CHANGE);
}

/*Regular functions*/
void handleInterrupt(void) {
  currentMillis = hal_functionToGetMillis();
}

static char binary_search(void)
{
    unsigned char left = 0u;
    unsigned char right = DEF_MORSETABLESZ - 1u;
    char outcome = (char)-1;

    while (left <= right) {
        unsigned char mid = (left + right) / (unsigned char)2u;
        if (st_morsetable[mid].value == value) {
            outcome = (char)mid;
        }
        else if (st_morsetable[mid].value < value) {
            left = mid + 1u;
        }
        else {
            right = mid - 1u;
        }
    }

    return outcome;
}

static unsigned short replaceBit(unsigned short n, unsigned short m, unsigned short i)
{
    unsigned short mask = (unsigned short)((unsigned short)1u << (unsigned short)i);
    return (n & ~mask) | (m << i);
}

static void printachar(void)
{
  char outcome;

  if((value >= st_morsetable[0].value) && (value <= st_morsetable[DEF_MORSETABLESZ-1u].value))
  {
    outcome = binary_search();

    if(outcome != (char)-1)
    {
      hal_functionToPrint(st_morsetable[(unsigned char)outcome].character);
      value = 0;
      idx = 0;
    }
    else
    {
      value = 0;
      idx = 0;
    }
  }
}

/*classic program structure*/
void setup(void) {
  hal_setup(DEF_INTERRUPTION_PIN);
  currentMillis = hal_functionToGetMillis();
  prev_currentMillis = hal_functionToGetMillis();
}

void loop() {
  static unsigned char state = DEF_STATE_LOW;
  static unsigned char counter = 0u;
  if(currentMillis != prev_currentMillis)
  {
    switch(state)
    {
      case DEF_STATE_LOW:
      {
        if((currentMillis - prev_currentMillis)>(DEF_MS_BETWEEN_WORDS))
        {
          printachar();
          hal_functionToPrint(' ');
        }
        else if((currentMillis - prev_currentMillis)>(DEF_MS_BETWEEN_CHARS))
        {
          printachar();
        }
        else
        {
            /*do nothing*/
        }

        state = DEF_STATE_HIGH;
      }break;
      case DEF_STATE_HIGH:
      {
        if((currentMillis - prev_currentMillis)<(DEF_MS_DOT))
        {
          //dot
          value = replaceBit(value,1u,(DEF_VALUEVAR_BITS-1u)-(unsigned short)idx);
          idx++;
          value = replaceBit(value,0u,(DEF_VALUEVAR_BITS-1u)-(unsigned short)idx);
          idx++;
        }
        else
        {
          //dash
          value = replaceBit(value,1u,(DEF_VALUEVAR_BITS-1u)-(unsigned short)idx);
          idx++;
          value = replaceBit(value,1u,(DEF_VALUEVAR_BITS-1u)-(unsigned short)idx);
          idx++;
          value = replaceBit(value,0u,(DEF_VALUEVAR_BITS-1u)-(unsigned short)idx);
          idx++;
        }
        state = DEF_STATE_LOW;
      }break;
      default:
      {
          /*do nothing*/
      }break;
    }
    prev_currentMillis = currentMillis;
  }
  else
  {
    if(DEF_STATE_LOW == state)
    {
      counter++;

      if(0xFFu == counter)
      {
        counter = (DEF_MS_MAX_WAIT_LOW)*2u;
      }

      if((counter>(DEF_MS_MAX_WAIT_LOW)) && (counter < ((DEF_MS_MAX_WAIT_LOW)*2u)))
      {
        printachar();
        counter = (DEF_MS_MAX_WAIT_LOW)*2u;
      }
    }
    else
    {
      counter = 0u;
    }
  }

  hal_delay(DEF_DELAYDURATION);
}