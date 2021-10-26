#include <iom8.h>
#include <ina90.h>
#include <string.h>
#include <pgmspace.h>
#include <stdio.h>
#include "main.h"
#include "USART.h"
#include "Monitor.h"
#include "Wrk_params.h"
#include "Timers.h"
#include "..\PRG_Transmitter\RF_settings.h"
#include "RF_receiver.h"
#include "Util.h"
#include "bin_defines.h"
#include "lcd.h"

#define LCD_CTRL_PORT PORTC
#define LCD_CTRL_DDR  DDRC

#define LCD_DATA_POUT PORTD
#define LCD_DATA_DDR  DDRD
#define LCD_DATA_PIN  PIND

#define LCD_CTRL_RS   P_RS
#define LCD_CTRL_RW   P_RW
#define LCD_CTRL_E    P_E

#define sbi(ADDRESS,BIT)   ((ADDRESS) |= (BIT))
#define cbi(ADDRESS,BIT)   ((ADDRESS) &= ~(BIT))
#define outb(ADDRESS,BYTE) ((ADDRESS) = BYTE)
#define inb(ADDRESS)        (ADDRESS)


#define LCD_LINE0_DDRAMADDR 0
#define LCD_LINE1_DDRAMADDR 40
#define LCD_LINE2_DDRAMADDR 0
#define LCD_LINE3_DDRAMADDR 40

#define LCD_DELAY   asm ("nop"); asm ("nop")

void lcdInitHW(void)
{
  // initialize I/O ports
  // if I/O interface is in use
  // initialize LCD control lines
  cbi(LCD_CTRL_PORT, LCD_CTRL_RS);
  cbi(LCD_CTRL_PORT, LCD_CTRL_RW);
  cbi(LCD_CTRL_PORT, LCD_CTRL_E);
  // initialize LCD control lines to output
  sbi(LCD_CTRL_DDR, LCD_CTRL_RS);
  sbi(LCD_CTRL_DDR, LCD_CTRL_RW);
  sbi(LCD_CTRL_DDR, LCD_CTRL_E);
  // initialize LCD data port to input
  // initialize LCD data lines to pull-up
  outb(LCD_DATA_DDR,  inb(LCD_DATA_DDR) & 0x0F);   // set data I/O lines to input (4bit)
  outb(LCD_DATA_POUT, inb(LCD_DATA_POUT)| 0xF0);   // set pull-ups to on (4bit)
}

void lcdBusyWait(void)
{
  // wait until LCD busy bit goes to zero
  // do a read from control register
  cbi(LCD_CTRL_PORT, LCD_CTRL_RS);        // set RS to "control"
  outb(LCD_DATA_DDR, inb(LCD_DATA_DDR)  & 0x0F); // set data I/O lines to input (4bit)
  outb(LCD_DATA_POUT, inb(LCD_DATA_POUT)| 0xF0); // set pull-ups to on (4bit)
  sbi(LCD_CTRL_PORT, LCD_CTRL_RW);        // set R/W to "read"
  sbi(LCD_CTRL_PORT, LCD_CTRL_E);         // set "E" line
  LCD_DELAY;                // wait
  while(inb(LCD_DATA_PIN) & 1<<LCD_BUSY)
  {
    cbi(LCD_CTRL_PORT, LCD_CTRL_E);   // clear "E" line
    LCD_DELAY;                  // wait
    LCD_DELAY;                  // wait
    sbi(LCD_CTRL_PORT, LCD_CTRL_E);   // set "E" line
    LCD_DELAY;                  // wait
    LCD_DELAY;                  // wait
    cbi(LCD_CTRL_PORT, LCD_CTRL_E); // clear "E" line
    LCD_DELAY;                // wait
    LCD_DELAY;                // wait
    sbi(LCD_CTRL_PORT, LCD_CTRL_E); // set "E" line
    LCD_DELAY;                // wait
    LCD_DELAY;                // wait
  }
  cbi(LCD_CTRL_PORT, LCD_CTRL_E);     // clear "E" line
  //  leave data lines in input mode so they can be most easily used for other purposes
}

void lcdControlWrite(unsigned char data)
{
// write the control byte to the display controller
  lcdBusyWait();              // wait until LCD not busy
  cbi(LCD_CTRL_PORT, LCD_CTRL_RS);      // set RS to "control"
  cbi(LCD_CTRL_PORT, LCD_CTRL_RW);      // set R/W to "write"
    // 4 bit write
  sbi(LCD_CTRL_PORT, LCD_CTRL_E); // set "E" line
  outb(LCD_DATA_DDR, inb(LCD_DATA_DDR)|0xF0); // set data I/O lines to output (4bit)
  outb(LCD_DATA_POUT, (inb(LCD_DATA_POUT)&0x0F) | (data&0xF0) );  // output data, high 4 bits
  LCD_DELAY;                // wait
  LCD_DELAY;                // wait
  cbi(LCD_CTRL_PORT, LCD_CTRL_E); // clear "E" line
  LCD_DELAY;                // wait
  LCD_DELAY;                // wait
  sbi(LCD_CTRL_PORT, LCD_CTRL_E); // set "E" line
  outb(LCD_DATA_POUT, (inb(LCD_DATA_POUT)&0x0F) | (data<<4) );  // output data, low 4 bits
  LCD_DELAY;                // wait
  LCD_DELAY;                // wait
  cbi(LCD_CTRL_PORT, LCD_CTRL_E); // clear "E" line
  //  leave data lines in input mode so they can be most easily used for other purposes
  outb(LCD_DATA_DDR, inb(LCD_DATA_DDR)&0x0F);   // set data I/O lines to input (4bit)
  outb(LCD_DATA_POUT, inb(LCD_DATA_POUT)|0xF0); // set pull-ups to on (4bit)
}

unsigned char lcdControlRead(void)
{
// read the control byte from the display controller
  register unsigned char data;
  lcdBusyWait();        // wait until LCD not busy
  outb(LCD_DATA_DDR, inb(LCD_DATA_DDR)&0x0F);   // set data I/O lines to input (4bit)
  outb(LCD_DATA_POUT, inb(LCD_DATA_POUT)|0xF0); // set pull-ups to on (4bit)
  cbi(LCD_CTRL_PORT, LCD_CTRL_RS);    // set RS to "control"
  sbi(LCD_CTRL_PORT, LCD_CTRL_RW);    // set R/W to "read"
  // 4 bit read
  sbi(LCD_CTRL_PORT, LCD_CTRL_E); // set "E" line
  LCD_DELAY;            // wait
  LCD_DELAY;            // wait
  data = inb(LCD_DATA_PIN)&0xF0;  // input data, high 4 bits
  cbi(LCD_CTRL_PORT, LCD_CTRL_E); // clear "E" line
  LCD_DELAY;            // wait
  LCD_DELAY;            // wait
  sbi(LCD_CTRL_PORT, LCD_CTRL_E); // set "E" line
  LCD_DELAY;            // wait
  LCD_DELAY;            // wait
  data |= inb(LCD_DATA_PIN)>>4; // input data, low 4 bits
  cbi(LCD_CTRL_PORT, LCD_CTRL_E); // clear "E" line
  //  leave data lines in input mode so they can be most easily used for other purposes
  return data;
}

void lcdDataWrite(unsigned char data)
{
// write a data byte to the display
  lcdBusyWait();              // wait until LCD not busy
  sbi(LCD_CTRL_PORT, LCD_CTRL_RS);    // set RS to "data"
  cbi(LCD_CTRL_PORT, LCD_CTRL_RW);    // set R/W to "write"
  // 4 bit write
  sbi(LCD_CTRL_PORT, LCD_CTRL_E); // set "E" line
  outb(LCD_DATA_DDR, inb(LCD_DATA_DDR)|0xF0); // set data I/O lines to output (4bit)
  outb(LCD_DATA_POUT, (inb(LCD_DATA_POUT)&0x0F) | (data&0xF0) );  // output data, high 4 bits
  LCD_DELAY;                // wait
  LCD_DELAY;                // wait
  cbi(LCD_CTRL_PORT, LCD_CTRL_E); // clear "E" line
  LCD_DELAY;                // wait
  LCD_DELAY;                // wait
  sbi(LCD_CTRL_PORT, LCD_CTRL_E); // set "E" line
  outb(LCD_DATA_POUT, (inb(LCD_DATA_POUT)&0x0F) | (data<<4) );  // output data, low 4 bits
  LCD_DELAY;                // wait
  LCD_DELAY;                // wait
  cbi(LCD_CTRL_PORT, LCD_CTRL_E); // clear "E" line
  //  leave data lines in input mode so they can be most easily used for other purposes
  outb(LCD_DATA_DDR, inb(LCD_DATA_DDR)&0x0F);   // set data I/O lines to input (4bit)
  outb(LCD_DATA_POUT, inb(LCD_DATA_POUT)|0xF0); // set pull-ups to on (4bit)
}

unsigned char lcdDataRead(void)
{
// read a data byte from the display
  register unsigned char data;
  lcdBusyWait();        // wait until LCD not busy
  outb(LCD_DATA_DDR, inb(LCD_DATA_DDR)&0x0F);   // set data I/O lines to input (4bit)
  outb(LCD_DATA_POUT, inb(LCD_DATA_POUT)|0xF0); // set pull-ups to on (4bit)
  sbi(LCD_CTRL_PORT, LCD_CTRL_RS);    // set RS to "data"
  sbi(LCD_CTRL_PORT, LCD_CTRL_RW);    // set R/W to "read"
  // 4 bit read
  sbi(LCD_CTRL_PORT, LCD_CTRL_E); // set "E" line
  LCD_DELAY;                // wait
  LCD_DELAY;                // wait
  data = inb(LCD_DATA_PIN)&0xF0;  // input data, high 4 bits
  cbi(LCD_CTRL_PORT, LCD_CTRL_E); // clear "E" line
  LCD_DELAY;                // wait
  LCD_DELAY;                // wait
  sbi(LCD_CTRL_PORT, LCD_CTRL_E); // set "E" line
  LCD_DELAY;                // wait
  LCD_DELAY;                // wait
  data |= inb(LCD_DATA_PIN)>>4;     // input data, low 4 bits
  cbi(LCD_CTRL_PORT, LCD_CTRL_E); // clear "E" line
  //  leave data lines in input mode so they can be most easily used for other purposes
  return data;
}



/*************************************************************/
/********************* PUBLIC FUNCTIONS **********************/
/*************************************************************/

void lcdInit()
{
  // initialize hardware
  lcdInitHW();
  // LCD function set
  lcdControlWrite(LCD_FUNCTION_DEFAULT);
  // clear LCD
  lcdControlWrite(1<<LCD_CLR);
  // set entry mode
  lcdControlWrite(1<<LCD_ENTRY_MODE | 1<<LCD_ENTRY_INC);
  // set display to on
  //lcdControlWrite(1<<LCD_ON_CTRL | 1<<LCD_ON_DISPLAY | 1<<LCD_ON_BLINK);
  lcdControlWrite(1<<LCD_ON_CTRL | 1<<LCD_ON_DISPLAY );
  // move cursor to home
  lcdControlWrite(1<<LCD_HOME);
  // set data address to 0
  lcdControlWrite(1<<LCD_DDRAM | 0x00);
}

void lcdHome(void)
{
  // move cursor to home
  lcdControlWrite(1<<LCD_HOME);
}

void lcdClear(void)
{
  // clear LCD
  lcdControlWrite(1<<LCD_CLR);
}

void lcdGotoXY(unsigned char x, unsigned char y)
{
  register unsigned char DDRAMAddr;

  // remap lines into proper order
  switch(y)
  {
  case 0: DDRAMAddr = LCD_LINE0_DDRAMADDR+x; break;
  case 1: DDRAMAddr = LCD_LINE1_DDRAMADDR+x; break;
  case 2: DDRAMAddr = LCD_LINE2_DDRAMADDR+x; break;
  case 3: DDRAMAddr = LCD_LINE3_DDRAMADDR+x; break;
  default: DDRAMAddr = LCD_LINE0_DDRAMADDR+x;
  }

  // set data address
  lcdControlWrite(1<<LCD_DDRAM | DDRAMAddr);
}


void lcdPrintData(char* data, unsigned char nBytes)
{
  register unsigned char i;

  // check to make sure we have a good pointer
  if (!data) return;

  // print data
  for(i=0; i<nBytes; i++)
  {
    lcdDataWrite(data[i]);
  }
}

void lcdPrintData_P(char __flash* data)
{
  unsigned char ch;

  // print data
  do
  {
    ch = *data++;
    if (ch==0) break;
    lcdDataWrite(ch);
  }
  while (1);
}



