PWM-4-Channel-1A
================

Original embedded firmware for the PWM 4-Channel controller with the PIC16F1829.

The 4-Channel PWM Controller can be used with low-voltage lighting or LEDs to control the light intensity, 
fading effect and duration with four independent channels with both user-definable light segments and macros. 
Some additional features include:
  
  * Up to 1000 user-definable light segments where each segment is composed of one or more sequences stored in 32KB EEPROM; 
  * Up to 100 macros that can include any combination of user light segments;
  * Each sequence has adjustable fade rates and hold durations along with independent intensity control for each channel; 
  * Automatic turn-on at dusk and turn-off at dawn with programmable on/off delays and duration;
  * Each channel operates from 8-20V at up to 5A with independent, hardware-based 10-bit PWM channel modulation;
  *  Modbus-like addressable ASCII protocol over an RS-485 wired interface to initiate/program/delete/read the light segments or macros and set/query the operating mode;
  * On-board multi-colour LED feedback indicators to show the active sequence being played;
  *  Push-button interface to quickly set up and define macros for custom operation;
  * Low-power sleep mode initiated with the push-button interface;
  *  Expansion port for use with a future RF-based control channel.

Hardware can be purchased from Computer Inspirations at www.c-inspirations.com.
