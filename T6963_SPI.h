/*
 ___  _   _  _ __ ___    ___  | |_  ___   _   _ 
/ __|| | | || '_ ` _ \  / _ \ | __|/ _ \ | | | |
\__ \| |_| || | | | | || (_) || |_| (_) || |_| |
|___/ \__,_||_| |_| |_| \___/  \__|\___/  \__, |
                                          |___/ 
										  
	easyT6963 - A Library & Hardware for drive Graphic LCD displays drived by T6963 controller with a GPIO (4 wires)
	------------- GPIO Low Level Routines -------------------
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
#ifndef T6963_SPI_H
#define T6963_SPI_H

#include <inttypes.h>
#include "easyT6963.h"
#include <../SPI/SPI.h>//this chip needs SPI
#include <../gpio_expander/mcp23s17.h>
#include "utility/_gpioConstants.h"//load gpio constants
#define nop asm volatile ("nop\n\t")

class T6963_SPI : public easyT6963 {
	public:
		/*
		specify CS pin, HAEN address and....
		*/
		T6963_SPI(const byte cs,const byte adrs=0x20,const byte avoidSPIinit=0);//instance
		virtual void 		begin(uint8_t width, uint8_t height, uint8_t charsize = T6963_6x8DOTS,uint8_t memsize = 32);
		virtual void 		setBacklight(uint8_t value);
		virtual void		hardwareFontSelect(bool val);//1:6*8, 0:8*8
		virtual void		hardwareScreenReverse(bool val);//1:reverse
	protected:
		virtual void 		sendCommand(byte command);
		virtual void 		sendData(byte data);
		virtual uint8_t 	readData(void);
		virtual uint8_t 	checkState(void);
	private:
		void				chipWaitState();
		void 				hardwareReset();
		mcp23s17			_gpio;
		uint8_t				_csPin;
		uint8_t				_adrs;
		uint8_t				_protocolInitOverride;
		//some time tweaking for MCU processors and speed
#if defined(__MK20DX128__) || defined(__MK20DX256__)  || defined(__MKL26Z64__)
		#if (F_CPU == 168000000)
		inline void		T6963_WAIT_DELAY() { delayMicroseconds(6); }
		#elif (F_CPU == 144000000)
		inline void		T6963_WAIT_DELAY() { delayMicroseconds(5); }
		#elif (F_CPU == 120000000)
		inline void		T6963_WAIT_DELAY() { delayMicroseconds(4); }
		#elif (F_CPU == 96000000)
		inline void		T6963_WAIT_DELAY() { delayMicroseconds(3); }
		#elif (F_CPU == 48000000)
		inline void		T6963_WAIT_DELAY() { delayMicroseconds(2); }
		#elif (F_CPU == 24000000)
		inline void		T6963_WAIT_DELAY() { delayMicroseconds(1); }
		#endif
#elif defined(__32MX320F128H__) || defined(__32MX795F512L__) || (defined(ARDUINO) && defined(__arm__) && !defined(CORE_TEENSY))//chipkit uno, chipkit max, arduino DUE	
		inline void		T6963_WAIT_DELAY() { delayMicroseconds(2); }
#elif defined(ENERGIA) // LaunchPad, FraunchPad and StellarPad specific	
		#if (F_CPU >= 80000000)
		inline void		T6963_WAIT_DELAY() { delayMicroseconds(3); }
		#elif ((F_CPU >= 40000000) && (F_CPU <= 80000000))
		inline void		T6963_WAIT_DELAY() { delayMicroseconds(2); }
		#else
		inline void		T6963_WAIT_DELAY() { delayMicroseconds(1); }
		// I don't have those so...
		#endif
#else
		inline void		T6963_WAIT_DELAY() { delayMicroseconds(1); }
#endif
};


#endif
