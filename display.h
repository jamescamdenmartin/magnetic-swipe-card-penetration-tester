#ifndef Display_h
#define Display_h

#include <inttypes.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

//uint8_t charsize = LCD_5x8DOTS;

  void display_init(uint8_t fourbitmode);
   
  void display_begin(uint8_t cols, uint8_t rows, uint8_t charsize);

  void display_clear();
  void display_home();

  void display_noDisplay();
  void display_show();
  void display_noBlink();
  void display_blink();
  void display_noCursor();
  void display_cursor();
  void display_scrollDisplayLeft();
  void display_scrollDisplayRight();
  void display_leftToRight();
  void display_rightToLeft();
  void display_autoscroll();
  void display_noAutoscroll();

  void display_createChar(uint8_t, uint8_t[]);
  void display_setCursor(uint8_t, uint8_t);
  void display_write(uint8_t);
  void display_command(uint8_t);

  void display_send(uint8_t, uint8_t);
  void display_write4bits(uint8_t);
  void display_write8bits(uint8_t);
  void display_pulseEnable();
  
  int display_prints(const char *);
  int display_println(const char *);
  void display_printl(const char *, uint8_t length, uint8_t clearrestofline);
  
  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _initialized;

  uint8_t _numlines,_currline,_numcolumns;


#endif

