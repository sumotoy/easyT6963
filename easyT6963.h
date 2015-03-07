/*
 ___  _   _  _ __ ___    ___  | |_  ___   _   _ 
/ __|| | | || '_ ` _ \  / _ \ | __|/ _ \ | | | |
\__ \| |_| || | | | | || (_) || |_| (_) || |_| |
|___/ \__,_||_| |_| |_| \___/  \__|\___/  \__, |
                                          |___/ 
										  
	easyT6963 - A Library & Hardware for drive Graphic LCD displays drived by T6963 controller with a GPIO (4 wires)
	--------- Main Library --------------
---------------------------------------------------------------------------------------------------------------------
Version history:
0.1b3:First working version, tested with Mega2560
---------------------------------------------------------------------------------------------------------------------
		Copyright (c) 2014, s.u.m.o.t.o.y [sumotoy(at)gmail.com]
---------------------------------------------------------------------------------------------------------------------

    easyT6963 Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. You cannot use for commercial producs
	or military purposes!

    easyT6963 Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

	Small code portions from Adafruit (Adafruit_GFX).
	https://github.com/adafruit/Adafruit-GFX-Library
	Originated from the library created by Radosław Kwiecień
	http://code.google.com/p/arduino-t6963c/
	Bitmap Byte technique by http://www.holger-klabunde.de
	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	Version:0.1b3
	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	Attention! This library needs sumotoy gpio_expander library
	>>>>>>>>> https://github.com/sumotoy/gpio_expander <<<<<<<<<<
	-------------------------------------------------------------
	Hardware needed:
	schematics coming soon...
	-------------------------------------------------------------
	Pins used:
	LCD SIDE			MCP23S17 SIDE
	D0					I/O:0
	D1					I/O:1
	D2					I/O:2
	D3					I/O:3
	D4					I/O:4
	D5					I/O:5
	D6					I/O:6
	D7					I/O:7
	CE					I/O:8
	CD					I/O:9
	WR					I/O:10
	RD					I/O:11 
	FS					I/O:12 *note 1
	RV					I/O:13 *note 2
	RST					I/O:14
	Backlight			I/O:15
						Reset	->	Vdd
						A0		-> *note 3
						A1		-> *note 3
						A2		-> *note 3
						MOSI	-> mosi
						MISO	-> miso
						SCK		-> sck
						CS		-> software selectable
						GND		-> gnd
*note 1: If your LCD doesn't have this pin, forget it.
*note 2: If your LCD doesn't have this pin, forget it.
*note 3: Those pin select MCP23S17 addressing for HAEN feature of the chip that let you share all SPI pins (include CS) between 8 chips. Tied all pins to ground will
result as address 0x20.
------------------------------------------------------------------
LCD Contrast: Every LCD has it own way so refer to datasheet, btw most LCD based on T6963 uses a negative offset voltage and unfortunatly sometimes it -12 or -15 so you will need
a StepUp negative circuit since you are probably driving everithing at +5V!
*/
#ifndef EASY_T6963_H
#define EASY_T6963_H

#if defined(ENERGIA) // LaunchPad, FraunchPad and StellarPad specific
#include "Energia.h"
#else
#include "Arduino.h"
#endif
#include <inttypes.h>
#include <Print.h>
#include "utility/_font.h"


//pgmspace fixup
#if defined(__MK20DX128__) || defined(__MK20DX256__)  || defined(__MKL26Z64__)//teensy 3 or 3.1 or LC
#include <avr/pgmspace.h>//Teensy3 and AVR arduinos can use pgmspace.h
#ifdef PROGMEM
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))
#endif
#elif defined(__32MX320F128H__) || defined(__32MX795F512L__) || (defined(ARDUINO) && defined(__arm__) && !defined(CORE_TEENSY))//chipkit uno, chipkit max, arduino DUE	
	#ifndef __PGMSPACE_H_
	#define __PGMSPACE_H_ 1
	#define PROGMEM
	#define PGM_P  const char *
	#define PSTR(str) (str)
	#define pgm_read_byte_near(addr) pgm_read_byte(addr)
	#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
	#define pgm_read_word(addr) (*(const unsigned short *)(addr))
	#endif
#else
#include <avr/pgmspace.h>//Teensy3 and AVR arduinos can use pgmspace.h

#endif

#include "utility/_chipConstants.h"//load controller constants

#define swap(a, b) { int8_t t = a; a = b; b = t; }

/* 
- Mega2560 SPI connections
	50 (MISO)
	51 (MOSI)
	52 (SCK)
	53 (SS)
*/

enum MODE_SEL{
	OR 	    = 0,
	XOR 	= 1,
	AND 	= 2,  
	TEXT 	= 3
};

enum CGRAM_SEL{
	INT 	= 0,
	EXT 	= 1
};

enum TEXT_SEL{
	TEXT_ON = true,
	TEXT_OFF = false
};

enum GRAPHIC_SEL{
	GRAPHIC_ON 	= true,
	GRAPHIC_OFF = false
};

enum CURSOR_SEL{
	CURSOR_ON 	= true,
	CURSOR_OFF = false
};

enum CURSOR_BLINKING{
	CURSOR_BLINK = true,
	CURSOR_NORMAL = false
};

enum TEXT_ATTRIBUTE{
	NORMAL = 0,
	REVERSE = 1,
	INHIBIT = 2,
	NBLINK = 3,
	RBLINK = 4,
	IBLINK = 5
};

class easyT6963: public Print {
public:
	easyT6963();
	virtual void 	begin(uint8_t width,uint8_t height,uint8_t charsize = T6963_6x8DOTS,uint8_t memsize = 32) = 0;
	//------------------- Hardware Releated -----------------------
	virtual void 	setBacklight(uint8_t value) = 0;
	virtual void	hardwareFontSelect(bool val) = 0;//1:6*8, 0:8*8
	virtual void	hardwareScreenReverse(bool val) = 0;//1:reverse
	//------------------ Display Mode -----------------------------
	uint8_t 		setDispMode(bool TEXT_SEL,bool GRAPHIC_SEL,bool CURSOR_SEL,bool CURSOR_BLINKING);
	void 			clearDispMode();
	uint8_t 		setMode(uint8_t MODE_SEL, uint8_t CGRAM_SEL=0);
	void			fastMode(bool mode);//0:slow/1:fast
	//------------------- Draw Functions ---------------------------
	void 			clearGraphic(uint8_t fastMode=1);
	void 			clearCG();
	void 			drawPixel(uint8_t x,uint8_t y,bool color=1);
	void 			drawLine(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,bool color=1);
	void 			drawFastVLine(uint8_t x, uint8_t y,uint8_t h, bool color=1);
	void 			drawFastHLine(uint8_t x, uint8_t y,uint8_t w, bool color=1);
	void 			drawTriangle(uint8_t x0, uint8_t y0,uint8_t x1, uint8_t y1,uint8_t x2, uint8_t y2, bool color=1);
	void 			drawRect(uint8_t x,uint8_t y,uint8_t b,uint8_t a,bool color=1);
	void 			drawFillRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool color=1);
	void 			drawFastRect(uint8_t x,uint8_t y,uint8_t height,uint8_t bytewidth,uint8_t pattern);
	void 			drawCircle(uint8_t cx, uint8_t cy, uint8_t radius, bool color=1);
	//------------------- BitMap Functions --------------------------
	void 			drawBitmap(uint8_t x,uint8_t y, const uint8_t *PicName,uint8_t height,uint8_t bytewidth);
	void 			drawAnim(uint8_t x,uint8_t y, const uint8_t **string_table, int l,uint8_t height,uint8_t bytewidth);
	//------------------- Text Functions ----------------------------
	void 			clearText();
	uint8_t 		getTextCols();
	uint8_t			getFontWidth();
	void 			setCursor(uint8_t col, uint8_t row);
	uint8_t 		setTextAttrMode(uint8_t TEXT_ATTRIBUTE=0);//NORMAL,REVERSE,INHIBIT,NBLINK,RBLINK,IBLINK
	void 			defineCharacter(byte charCode, unsigned char * defChar);
    virtual size_t 	write(uint8_t value);
	using Print::write;
	//------------------- Graphic Cursors --------------------------
	void 			graphicGoTo(uint8_t x, uint8_t y);
	byte 			setCursorPattern(uint8_t b);
	void 			setCursorPointer(uint8_t col,uint8_t row);//VERIFICA!
	//uint8_t 		gWrite(uint8_t x,uint8_t y,char in, const struct FONT_DEF *strcut1,bool color);
	void  			gPrint(uint8_t x,uint8_t y, const char *in,  const struct FONT_DEF *strcut1,bool color);
	

protected:
	void 			setTextHome(unsigned int addr);
	void			setGraphicHome(unsigned int addr);
	void 			plot8points(uint8_t cx, uint8_t cy, uint8_t x, uint8_t y, bool color);
	void 			plot4points(uint8_t cx, uint8_t cy, uint8_t x, uint8_t y, bool color);
	/* --------------------------------------- shared protected vars -------------------------------*/
	uint8_t					_width;
	uint8_t					_height;
	uint8_t					_fontWidth;
	int						_memSize;
	unsigned int			_graphicArea;
	unsigned int			_graphicSize;
	unsigned int			_textSize;
	unsigned int			_textHome;
	unsigned int			_graphicHome;
	unsigned int			_maxCol;
	unsigned int			_maxRow;
	
	uint8_t					_hrdwFS;
	uint8_t					_hrdwRV;
	uint8_t		    		_backlight;
	uint8_t					_fastMode;
	
	volatile uint8_t		_gx;
	volatile uint8_t		_gy;
	
private:
	void 			setAddressPointer(unsigned int address);
	void 			writeDisplayData(byte x);
	void 			writeDataNon(byte x);
	void 			writeDisplayDataDec(byte x);
	//uint8_t 		strlenght(const char *string);
	unsigned int 	calculateAddress(uint8_t x, uint8_t y);
	virtual void 	sendCommand(byte command) = 0;
	virtual void 	sendData(byte data) = 0;
	virtual byte 	readData(void) = 0;
	virtual byte 	checkState(void) = 0;


    
};


#endif
