/*
*********************************************************************************************************
* Include Header Files
*********************************************************************************************************
*/
#include <reg52.h>                                     // microcontroller header file
#include "max7219.h"                                  // MAX7219 header file


/*
*********************************************************************************************************
* Constants
*********************************************************************************************************
*/
#define REG_DECODE        0x09                        // "decode mode" register
#define REG_INTENSITY     0x0a                        // "intensity" register
#define REG_SCAN_LIMIT    0x0b                        // "scan limit" register
#define REG_SHUTDOWN      0x0c                        // "shutdown" register
#define REG_DISPLAY_TEST  0x0f                        // "display test" register

#define INTENSITY_MIN     0x00                        // minimum display intensity
#define INTENSITY_MAX     0x0f                        // maximum display intensity


/*
*********************************************************************************************************
* Macros
*********************************************************************************************************
*/
#define DATA_PORT     P3                             // "DIN" is on P3.5
#define DATA_DDR      P3
#define DATA_BIT      0x20
#define DATA_0()      (DATA_PORT &= ~DATA_BIT)
#define DATA_1()      (DATA_PORT |=  DATA_BIT)
#define CLK_PORT      P3                              // "CLK" is on P3.4
#define CLK_DDR       P3
#define CLK_BIT       0x10
#define CLK_0()       (CLK_PORT &= ~CLK_BIT)
#define CLK_1()       (CLK_PORT |=  CLK_BIT)
#define LOAD_PORT     P3                              // "LOAD - CS" is on P3.3
#define LOAD_DDR      P3
#define LOAD_BIT      0x08
#define LOAD_0()      (LOAD_PORT &= ~LOAD_BIT)
#define LOAD_1()      (LOAD_PORT |=  LOAD_BIT)


/*
*********************************************************************************************************
* Private Data
*********************************************************************************************************
*/

char status[8];

/*
*********************************************************************************************************
* Private Function Prototypes
*********************************************************************************************************
*/
static void MAX7219_Write(unsigned char reg_number, unsigned char data_);
static void MAX7219_SendByte(unsigned char data_);


// ...................................... Public Functions ..............................................


/*
*********************************************************************************************************
* MAX7219_Init()
*
* Description: Initialize MAX7219 module; must be called before any other MAX7219 functions.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_Init (void)
{
  DATA_DDR |= DATA_BIT;                               // configure "DATA" as output
  CLK_DDR  |= CLK_BIT;                                // configure "CLK"  as output
  LOAD_DDR |= LOAD_BIT;                               // configure "LOAD" as output

  MAX7219_Write(REG_SCAN_LIMIT, 7);                   // set up to scan all eight digits
  MAX7219_Write(REG_DECODE, 0x00);                    // set to "no decode" for all digits
  MAX7219_ShutdownStop();                             // select normal operation (i.e. not shutdown)
  MAX7219_DisplayTestStop();                          // select normal operation (i.e. not test mode)
  MAX7219_Clear();                                    // clear all digits
  MAX7219_SetBrightness(INTENSITY_MAX);               // set to maximum intensity
}


/*
*********************************************************************************************************
* MAX7219_ShutdownStop()
*
* Description: Take the display out of shutdown mode.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_ShutdownStop (void)
{
  MAX7219_Write(REG_SHUTDOWN, 1);                     // put MAX7219 into "normal" mode
}

/*
*********************************************************************************************************
* MAX7219_DisplayTestStop()
*
* Description: Stop a display test.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_DisplayTestStop (void)
{
  MAX7219_Write(REG_DISPLAY_TEST, 0);                 // put MAX7219 into "normal" mode
}


/*
*********************************************************************************************************
* MAX7219_SetBrightness()
*
* Description: Set the LED display brightness
* Arguments  : brightness (0-15)
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_SetBrightness (char brightness)
{
  brightness &= 0x0f;                                 // mask off extra bits
  MAX7219_Write(REG_INTENSITY, brightness);           // set brightness
}


/*
*********************************************************************************************************
* MAX7219_Clear()
*
* Description: Clear the display (all digits blank)
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_Clear (void)
{
  char i;
  for (i=0; i < 8; i++){
		status[i] = 0x00;
    MAX7219_Write(i+1, status[i]);                           // turn all segments off
	}
}


// ..................................... Private Functions ..............................................

/*
*********************************************************************************************************
* MAX7219_Write()
*
* Description: Write to MAX7219
* Arguments  : reg_number = register to write to
*              dataout = data to write to MAX7219
* Returns    : none
*********************************************************************************************************
*/
static void MAX7219_Write (unsigned char reg_number, unsigned char dataout)
{
  LOAD_1();                                           // take LOAD high to begin
  MAX7219_SendByte(reg_number);        // write register number to MAX7219
  MAX7219_SendByte(dataout);              // write data to MAX7219
  LOAD_0();                                           // take LOAD low to latch in data
  LOAD_1();                                           // take LOAD high to end
}


/*
*********************************************************************************************************
* MAX7219_SendByte()
*
* Description: Send one byte to the MAX7219
* Arguments  : dataout = data to send
* Returns    : none
*********************************************************************************************************
*/
static void MAX7219_SendByte (unsigned char dataout)
{
  char i;
  for (i=8; i>0; i--) {
    unsigned char mask = 1 << (i - 1);                // calculate bitmask
    CLK_0();                                          // bring CLK low
    if (dataout & mask)                               // output one data bit
      DATA_1();                                       //  "1"
    else                                              //  or
      DATA_0();                                       //  "0"
    CLK_1();                                          // bring CLK high
	}
}

/** Custom functions */
void setLed(int row, int col, int state){
	char val;
	if(row<0 || row>7 || col<0 || col>7)
		return;
	val = 0x00;
	col += 1;
	val = 256>>col;
	if(state==1)
		status[row] = status[row]|val;
	else{
		val = ~val;
		status[row] = status[row]&val;
	}
	MAX7219_Write(row+1, status[row]);
}

void setRow(int row, char val){
	if(row<0 || row>7)
		return;
	status[row] = val;
	MAX7219_Write(row+1, status[row]);
}

/** set entire screen (row wise)*/
void setScreen(char* rows){
	int i;
	for(i=0;i<8;i++){
		setRow(i, rows[i]);
	}
}

