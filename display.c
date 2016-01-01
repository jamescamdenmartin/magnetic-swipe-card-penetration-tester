#include "display.h"
#include "globaldefinitions.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

//Macros for easier IO Pin handling
/*#define _BSET(type,name,bit)            (type ## name  |= _BV(bit))   
#define _BCLEAR(type,name,bit)          (type ## name  &= ~ _BV(bit))       
#define _BTOGGLE(type,name,bit)         (type ## name  ^= _BV(bit))   
#define _BGET(type,name,bit)            ((type ## name >> bit) &  1)
#define _BPUT(type,name,bit,val)      (type ## name = ( type ## name & ( ~ _BV(bit)) ) | ( ( 1 & (unsigned char)val ) << bit ))

//these macros are used by end user
#define OUTPUT(pin)            _BSET(DDR,pin)   
#define INPUT(pin)             _BCLEAR(DDR,pin)   
#define HIGH(pin)              _BSET(PORT,pin)
#define LOW(pin)               _BCLEAR(PORT,pin)   
#define TOGGLE(pin)            _BTOGGLE(PORT,pin)   
#define READ(pin)              _BGET(PIN,pin)
#define PUT(pin,val)           _BPUT(PORT,pin,val)
*/

#define _rs_pin D,6
#define _rw_pin B,255
#define _enable_pin D,7
#define _d0_pin B,4
#define _d1_pin B,3
#define _d2_pin B,2
#define _d3_pin B,0
#define _d4_pin C,1
#define _d5_pin C,1
#define _d6_pin C,1
#define _d7_pin C,1

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//

void display_init(uint8_t fourbitmode)
{
  /*_rs_pin = rs;
  _rw_pin = rw;
  _enable_pin = enable;
 
  _data_pins[0] = d0;
  _data_pins[1] = d1;
  _data_pins[2] = d2;
  _data_pins[3] = d3;
  _data_pins[4] = d4;
  _data_pins[5] = d5;
  _data_pins[6] = d6;
  _data_pins[7] = d7;*/

  OUTPUT(_rs_pin);
  // we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
  /*if (_rw_pin != 255) {
    OUTPUT(_rw_pin);
  }*/
  OUTPUT(_enable_pin);
 
  if (fourbitmode)
    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  else
    _displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
 
  display_begin(16, 2, LCD_5x8DOTS);  
}

void display_begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;
  _numcolumns = cols;
  _currline = 0;

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != 0) && (lines == 1)) {
    _displayfunction |= LCD_5x10DOTS;
  }

  // according to datasheet, we need at least 40ms after the input to the display rises above 2.7V
  _delay_us(50000);
  // Now we pull both RS and R/W low to begin display_commands
  LOW(_rs_pin);
  LOW(_enable_pin);
/*  if (_rw_pin != 255) {
    LOW(_rw_pin);
  }*/
 
  //put the LCD into 4 bit or 8 bit mode
  if (! (_displayfunction & LCD_8BITMODE)) {
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    display_write4bits(0x03);
    _delay_us(4500); // wait min 4.1ms

    // second try
    display_write4bits(0x03);
    _delay_us(4500); // wait min 4.1ms
   
    // third go!
    display_write4bits(0x03);
    _delay_us(150);

    // finally, set to 8-bit interface
    display_write4bits(0x02);
  } else {
    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set display_command sequence
    display_command(LCD_FUNCTIONSET | _displayfunction);
    _delay_us(4500);  // wait more than 4.1ms

    // second try
    display_command(LCD_FUNCTIONSET | _displayfunction);
    _delay_us(150);

    // third go
    display_command(LCD_FUNCTIONSET | _displayfunction);
  }

  // finally, set # lines, font size, etc.
  display_command(LCD_FUNCTIONSET | _displayfunction);  

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display_show();

  // clear it off
  display_clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  display_command(LCD_ENTRYMODESET | _displaymode);

}

/********** high level display_commands, for the user! */
void display_clear()
{
  display_command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  _delay_us(2000);  // this display_command takes a long time!
}

void display_home()
{
  display_command(LCD_RETURNHOME);  // set cursor position to zero
  _delay_us(2000);  // this display_command takes a long time!
}

void display_setCursor(uint8_t col, uint8_t row)
{
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if ( row > _numlines ) {
    row = _numlines-1;    // we count rows starting w/0
  }
 
  display_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void display_noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  display_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void display_show() {
  _displaycontrol |= LCD_DISPLAYON;
  display_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void display_noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  display_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void display_cursor() {
  _displaycontrol |= LCD_CURSORON;
  display_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void display_noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  display_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void display_blink() {
  _displaycontrol |= LCD_BLINKON;
  display_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These display_commands scroll the display without changing the RAM
void display_scrollDisplayLeft(void) {
  display_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void display_scrollDisplayRight(void) {
  display_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void display_leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  display_command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void display_rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  display_command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void display_autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  display_command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void display_noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  display_command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void display_createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  display_command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    display_write(charmap[i]);
  }
}

/************ low level data pushing display_commands **********/

// write either display_command or data, with automatic 4/8-bit selection
void display_senddisplay_command(uint8_t value) {
  LOW(_rs_pin);

  // if there is a RW pin indicated, set it low to Write
 /* if (_rw_pin != 255) {
    LOW(_rw_pin);
  }*/
 
  if (_displayfunction & LCD_8BITMODE) {
    display_write8bits(value);
  } else {
    display_write4bits(value>>4);
    display_write4bits(value);
  }
}

void display_sendtext(uint8_t value) {
  HIGH(_rs_pin);

  // if there is a RW pin indicated, set it low to Write
/*  if (_rw_pin != 255) {
    LOW(_rw_pin);
  }*/
 
  if (_displayfunction & LCD_8BITMODE) {
    display_write8bits(value);
  } else {
    display_write4bits(value>>4);
    display_write4bits(value);
  }
}

/*********** mid level display_commands, for sending data/cmds */

inline void display_command(uint8_t value) {
	display_senddisplay_command(value);
}

inline void display_write(uint8_t value) {
	display_sendtext(value);
}


void display_pulseEnable(void) {
  LOW(_enable_pin);
  _delay_us(1);    
  HIGH(_enable_pin);
  _delay_us(1);    // enable pulse must be >450ns
  LOW(_enable_pin);
  _delay_us(100);   // display_commands need > 37us to settle
}

void display_write4bits(uint8_t value) {

    OUTPUT(_d0_pin);
	PUT(_d0_pin,(value >> 0) & 0x01);
    OUTPUT(_d1_pin);
	PUT(_d1_pin,(value >> 1) & 0x01);
    OUTPUT(_d2_pin);
	PUT(_d2_pin,(value >> 2) & 0x01);
    OUTPUT(_d3_pin);
	PUT(_d3_pin,(value >> 3) & 0x01);

  display_pulseEnable();
}

void display_write8bits(uint8_t value) {
    OUTPUT(_d0_pin);
	PUT(_d0_pin,(value >> 0) & 0x01);
    OUTPUT(_d1_pin);
	PUT(_d1_pin,(value >> 1) & 0x01);
    OUTPUT(_d2_pin);
	PUT(_d2_pin,(value >> 2) & 0x01);
    OUTPUT(_d3_pin);
	PUT(_d3_pin,(value >> 3) & 0x01);
    OUTPUT(_d4_pin);
	PUT(_d4_pin,(value >> 4) & 0x01);
    OUTPUT(_d5_pin);
	PUT(_d5_pin,(value >> 5) & 0x01);
    OUTPUT(_d6_pin);
	PUT(_d6_pin,(value >> 6) & 0x01);
    OUTPUT(_d7_pin);
	PUT(_d7_pin,(value >> 7) & 0x01);
 
  display_pulseEnable();
}

/***************
Writes out to a line, and fills the rest of it with blanks
***************/
uint8_t display_println(const char *str)
{
	uint8_t i=0;
	while(1){
		if(str[i]==0)
		break;
		display_write(str[i]);
		i++;
	}
	if(i<_numcolumns)
		for(uint8_t j=0;j<_numcolumns-i;j++){
			display_write(' ');
		}
	return i;
}

uint8_t display_prints(const char *str)
{
	uint8_t i=0;
	while(1){
		if(str[i]==0)
			break;
		display_write(str[i]);
		i++;
	}
	return i;
}

void display_printl(const char *str, uint8_t length, uint8_t clearrestofline)
{
	for(uint8_t i=0; i<length; i++)
		display_write(str[i]);
	if(clearrestofline){
		for(uint8_t j=0;j<_numcolumns-length;j++){
			display_write(' ');
		}
	}
}