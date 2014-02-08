
#include <inttypes.h>
#include <Arduino.h>
#include "T6963_SPI.h"
#include <../SPI/SPI.h>//this chip needs SPI
#include <../gpio_expander/mcp23s17.h>



T6963_SPI::T6963_SPI(const byte cs,const byte adrs,const byte avoidSPIinit){
	_csPin = cs;
	_adrs = adrs;
	_protocolInitOverride = avoidSPIinit;
	
}

void T6963_SPI::setBacklight(uint8_t value){
	if (value > 1) value = 1;
	_backlight = value;
	uint16_t	gpioData = 0xFFFF;//start with this
	bitWrite(gpioData,GPIO__BLK,_backlight);//backlight
	_gpio.gpioPort(gpioData);//send data to GPIO
}

/*
default:true
*/
void T6963_SPI::hardwareFontSelect(bool val){
	_hrdwFS = val;	
	uint16_t	gpioData = 0xFFFF;//start with this
	bitWrite(gpioData,GPIO__FS,_hrdwFS);//Hardware font select
	_gpio.gpioPort(gpioData);//send data to GPIO
}

/*
true:reversed,false:normal
*/
void T6963_SPI::hardwareScreenReverse(bool val){
	_hrdwRV = !val;
	uint16_t	gpioData = 0xFFFF;//start with this 
	bitWrite(gpioData,GPIO__RV,_hrdwRV);//Hardware screen reverse
	_gpio.gpioPort(gpioData);//send data to GPIO
}

void T6963_SPI::hardwareReset(){
	uint16_t gpioData = 0xFFFF;//start with this
	bitClear(gpioData,GPIO__RST);//Hardware font select
	_gpio.gpioPort(gpioData);//send data to GPIO
	delay(700);
	bitSet(gpioData,GPIO__RST);//Hardware font select
	_gpio.gpioPort(gpioData);//send data to GPIO
	delay(100);
	setTextHome(0);
	setGraphicHome(0);
	//Set Graphics Home Address
	sendData(_graphicHome & 0xFF);
	sendData(_graphicHome >> 8);
	sendCommand(T6963_SET_GRAPHIC_HOME_ADDRESS);
	//Set Graphics Area
	sendData(_graphicArea);
	sendData(0x00);
	sendCommand(T6963_SET_GRAPHIC_AREA);
	//Set Text home address
	sendData(_textHome & 0xFF);
	sendData(_textHome >> 8);
	sendCommand(T6963_SET_TEXT_HOME_ADDRESS);
	//Set Text Area
	sendData(_graphicArea);
	sendData(0x00);
	sendCommand(T6963_SET_TEXT_AREA);
	//Set Internal CGRAM address
	sendData(((_memSize/2)-1));
	sendData(0x00);
	sendCommand(T6963_SET_OFFSET_REGISTER);
	setDispMode(TEXT_ON,GRAPHIC_ON,CURSOR_OFF,CURSOR_NORMAL);
	setMode(NORMAL,INT);
	clearText();
	clearGraphic();
	delay(10);
}

void T6963_SPI::begin(uint8_t width, uint8_t height, uint8_t charsize,uint8_t memsize){
	_backlight = T6963_BLK_DEFAULT;
	_hrdwFS = T6963_FS_DEFAULT;
	_hrdwRV = T6963_SREVERSE_DEFAULT;
	_width = width;							//240
	_height = height;						//128
	_fontWidth = charsize;					//6 or 8
	_memSize = constrain(memsize,0,64);		//32
	_graphicArea = _width / _fontWidth;		//40 	(6)
	_graphicSize = _graphicArea * _width;	//9600 	(6)
	//_textSize = _graphicSize / 8;			//1200	(6) maybe _graphicSize / _fontWidth;
	_textSize = _graphicSize / _fontWidth;
	_maxCol = _graphicArea - 1;				//39 	(6)
	//_maxRow = (_width / 8) - 1;			//29    (6) maybe (_width / _fontWidth) - 1;
	_maxRow = (_width / _fontWidth) - 1;
	_gpio.postSetup(_csPin,_adrs);//init external GPIO vars
	if (_protocolInitOverride){
		SPI.begin();
		SPI.setClockDivider(SPI_CLOCK_DIV2); // 4 MHz (half speed)
		SPI.setBitOrder(MSBFIRST);
		SPI.setDataMode(SPI_MODE0);
	}
	_gpio.begin(_protocolInitOverride);//init GPIO

	_gpio.gpioPinMode(OUTPUT);//All GPIO ports as out
	_gpio.gpioPort(0xFFFF);//All HIGH
	hardwareReset();
}

/*
	Low Level Hardware Routines
*/
void T6963_SPI::sendCommand(byte command){
	while(!(checkState() & 0x03));
	uint16_t gpioData = (0xFFFF << 8) | command;
	bitWrite(gpioData,GPIO__FS,_hrdwFS);//Hardware font select
	bitWrite(gpioData,GPIO__RV,_hrdwRV);//Hardware screen reverse
	bitWrite(gpioData,GPIO__BLK,_backlight);//backlight
	gpioData &= ~((1 << GPIO__WR) | (1 << GPIO__CE));//clear
	_gpio.gpioPort(gpioData);//send data to GPIO
	//nop;
	gpioData |= ((1 << GPIO__WR) | (1 << GPIO__CE)); //setta
	_gpio.gpioPort(gpioData);//send data to GPIO
}

void T6963_SPI::sendData(byte data){
	while(!(checkState() & 0x03));
	uint16_t gpioData = (0xFFFF << 8) | data;
	bitWrite(gpioData,GPIO__FS,_hrdwFS);//Hardware font select
	bitWrite(gpioData,GPIO__RV,_hrdwRV);//Hardware screen reverse
	bitWrite(gpioData,GPIO__BLK,_backlight);//backlight
	gpioData &= ~((1 << GPIO__CD) | (1 << GPIO__WR) | (1 << GPIO__CE));//clear
	_gpio.gpioPort(gpioData);//send data to GPIO
	//nop;
	gpioData |= ((1 << GPIO__CD) | (1 << GPIO__WR) | (1 << GPIO__CE)); //setta
	_gpio.gpioPort(gpioData);//send data to GPIO
}

uint8_t T6963_SPI::readData(void){
	uint8_t tmp = 0xFF;
	uint16_t gpioData = 0xFFFF;//start with this
	while(!(checkState() & 0x03));
	_gpio.gpioPinMode(0b0000000011111111);//port A=in, port B=out
	bitWrite(gpioData,GPIO__FS,_hrdwFS);//Hardware font select
	bitWrite(gpioData,GPIO__RV,_hrdwRV);//Hardware screen reverse
	bitWrite(gpioData,GPIO__BLK,_backlight);//backlight
	gpioData &= ~((1 << GPIO__CD) | (1 << GPIO__RD) | (1 << GPIO__CE));//clear
	_gpio.gpioPort(gpioData);//send data to GPIO
	//nop;
	uint16_t p = _gpio.readGpioPort();//now read port
	tmp = p & 0xFF;//get lower 8 bits
	_gpio.gpioPinMode(OUTPUT);//set ports as OUT as before
	gpioData |= ((1 << GPIO__CD) | (1 << GPIO__RD) | (1 << GPIO__CE)); //setta
	_gpio.gpioPort(gpioData);//send data to GPIO
	return tmp;
}

uint8_t T6963_SPI::checkState(void){
	uint8_t tmp = 0xFF;;
	uint16_t	gpioData = 0xFFFF;//start with this
	_gpio.gpioPinMode(0b0000000011111111);//port A=in, port B=out
	bitWrite(gpioData,GPIO__FS,_hrdwFS);//Hardware font select
	bitWrite(gpioData,GPIO__RV,_hrdwRV);//Hardware screen reverse
	bitWrite(gpioData,GPIO__BLK,_backlight);//backlight
	gpioData &= ~((1 << GPIO__RD) | (1 << GPIO__CE));//clear
	_gpio.gpioPort(gpioData);//send data to GPIO
	//nop;
	uint16_t p = _gpio.readGpioPort();//now read port
	tmp = p & 0xFF;//get lower 8 bits
	_gpio.gpioPinMode(OUTPUT);//set ports as OUT as before
	gpioData |= ((1 << GPIO__RD) | (1 << GPIO__CE)); //setta
	_gpio.gpioPort(gpioData);//send data to GPIO
	return tmp;
}

