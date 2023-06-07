
# Morse-Huffman Library

This library is a project to be used as tesis project to get the master degree at Universidad Autonoma de Guadalajara in Science Computer.

A C library to add to your project the capability of transmit at runtime messages in Morse format directly to the human eye using an output of the microcontoller and plus, using the same output transmit a character at each change of edge of the output using a Huffmancode in order to get compressed bits to be transmitted and doing that save time and avoid an intrusion to the main performance of the application.

This method is aiming to be used mostly in the academic context and some specific scenarios of industry.

## Authors

- [@danielrubio23](https://www.github.com/danubio23)


## Documentation

[Tesis document (spanish version)](https://github.com/danubio23/morsehuffman_lib/blob/main/Morse_Huffman_Tesis.pdf)


## Support

For support, email daniel.rubio23@gmail.com.


## API Reference

#### Init function

```http
  void init_MorseHuffman(unsigned char led_id);
```

| Parameter | Type     | Description                |
| :-------- | :------- | :------------------------- |
| `led_id` | `unsigned char` | **Required**. The ID of your GPIO to be used by the library |

Add this init function in the init phase of your project.

#### Main finite state machine call

```http
  void morsehuffman_fsm(void);
```

Call this function in a periodic task that runs in a fix period of time, so that time will be the base time unit for Morse transmisión (transmitting a dot uses a high pulse at the output of 1 time unit of duration, meanwhile a dash uses a high pulse of 3 time units of duration).

#### API to transmite data

```http
  void morsehuffman_msg(const char *str, op_modes mode);
```

| Parameter | Type     | Description                       |
| :-------- | :------- | :-------------------------------- |
| `str`      | `string` | **Required**. String to be transmitted |
| `mode`      | `op_modes` | Specify what method do you want to use: enum_opmode_morse (default one) or enum_opmode_huffman |

Each method (Morse and Huffman) has a independent buffer of configurable size, any invalid character or characters that doesn't fit in the avaiable space of the buffer selected, will be ignored.


## Installation

Befor to proceed with the deployment of the library in the project, please follow these 2 steps:


1. Confirm that the defines contained in morsehuffman_cfg.h file are matching with the configuration that you want to run the library. 

| Parameter | Type     | Description                |
| :-------- | :------- | :------------------------- |
| `def_cfg_buffer_size` | `define` | Bite size of buffers, 16 by default value |
| `def_cfg_huffman_us_0` | `define` | Microseconds to wait between pulses of Huffman transmision, 1 by default value |

2. Update the morsehuffman_hal.c file to fill all the functions with the proper API accordentely to the hardware.

#### HAL level for update the output state

```http
  void hal_switch_led(unsigned char state)
```

| Parameter | Type     | Description                |
| :-------- | :------- | :------------------------- |
| `state` | `unsigned char` | **Required**. 0 to get a falling edge change, any other value to get a rising edge change. |

Updates the state of the configured GPIO as main output of the library accordentely to the parameter received, is suggested to use the register directly in order to save time and make this transmision shorter and reliable.

#### HAL level for delay execution in Huffman transmision

```http
  void hal_delayexecution_us(unsigned char us) 
```
| Parameter | Type     | Description                       |
| :-------- | :------- | :-------------------------------- |
| `us`      | `unsigned char` | **Required**. amount of microseconds to delay the execution of the application. |

In Huffman data transmision we need to have different times of duration of the pulses to difference between transmit a 1 or a 0, so we need to delay the execution of the application different amount of microseconds depending the digit to be transmitted.

#### HAL level for Init the GPIO as output

```http
  void hal_setuppin(unsigned char led_id) 
```

| Parameter | Type     | Description                       |
| :-------- | :------- | :-------------------------------- |
| `led_id`      | `unsigned char` | **Required**. ID of the GPIO to be used as main output. |

Use any API required accordentely to the specific hardware where the code is running to setup the GPIO received as parameter as the main output of this library.
    
## Deployment

To deploy this project:

1. Add the Init function to a init function of the project.
2. Add the call of the main_fsm function to a periodic task of the project.
3. Use the API morsehuffman_msg at any moment that you need to transmit data using any of the 2 methods available.



## License

MIT License

Copyright (c) 2023 Daniel Rubio Martín del Campo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

