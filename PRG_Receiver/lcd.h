
#ifndef LCD_H
#define LCD_H

#include <iom8.h>
#include <ina90.h>

// HD44780 LCD controller command set (do not modify these)
// writing:
#define LCD_CLR             0      // DB0: clear display
#define LCD_HOME            1      // DB1: return to home position
#define LCD_ENTRY_MODE      2      // DB2: set entry mode
#define LCD_ENTRY_INC       1      //   DB1: increment
#define LCD_ENTRY_SHIFT     0      //   DB2: shift
#define LCD_ON_CTRL         3      // DB3: turn lcd/cursor on
#define LCD_ON_DISPLAY      2      //   DB2: turn display on
#define LCD_ON_CURSOR       1      //   DB1: turn cursor on
#define LCD_ON_BLINK        0      //   DB0: blinking cursor
#define LCD_MOVE            4      // DB4: move cursor/display
#define LCD_MOVE_DISP       3      //   DB3: move display (0-> move cursor)
#define LCD_MOVE_RIGHT      2      //   DB2: move right (0-> left)
#define LCD_FUNCTION        5      // DB5: function set
#define LCD_FUNCTION_8BIT   4      //   DB4: set 8BIT mode (0->4BIT mode)
#define LCD_FUNCTION_2LINES 3      //   DB3: two lines (0->one line)
#define LCD_FUNCTION_10DOTS 2      //   DB2: 5x10 font (0->5x7 font)
#define LCD_CGRAM           6      // DB6: set CG RAM address
#define LCD_DDRAM           7      // DB7: set DD RAM address
// reading:
#define LCD_BUSY            7      // DB7: LCD is busy

// Default LCD setup
// this default setup is loaded on LCD initialization
#define LCD_FDEF_1			  (0<<LCD_FUNCTION_8BIT)
#define LCD_FDEF_2				(1<<LCD_FUNCTION_2LINES)
#define LCD_FUNCTION_DEFAULT	((1<<LCD_FUNCTION) | LCD_FDEF_1 | LCD_FDEF_2)
#define LCD_MODE_DEFAULT		((1<<LCD_ENTRY_MODE) | (1<<LCD_ENTRY_INC))



// ****** Low-level functions ******
// the following functions are the only ones which deal with the CPU
// memory or port pins directly.  If you decide to use a fundamentally
// different hardware interface to your LCD, only these functions need
// to be changed, after which all the high-level functions will
// work again.

// initializes I/O pins connected to LCD
void lcdInitHW(void);
// waits until LCD is not busy
void lcdBusyWait(void);
// writes a control command to the LCD
void lcdControlWrite(unsigned char data);
// read the control status from the LCD
unsigned char lcdControlRead(void);
// writes a data byte to the LCD screen at the current position
void lcdDataWrite(unsigned char data);
// reads the data byte on the LCD screen at the current position
unsigned char lcdDataRead(void);


// ****** High-levlel functions ******
// these functions provide the high-level control of the LCD
// such as clearing the display, setting cursor positions,
// displaying text and special characters

// initializes the LCD display (gets it ready for use)
void lcdInit(void);

// moves the cursor/position to Home (upper left corner)
void lcdHome(void);

// clears the LCD display
void lcdClear(void);

// moves the cursor/position to the row,col requested
// ** this may not be accurate for all displays
void lcdGotoXY(unsigned char row, unsigned char col);


// prints a series of bytes/characters to the display
void lcdPrintData(char* data, unsigned char nBytes);

void lcdPrintData_P(char __flash* data);


#endif
