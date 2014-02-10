#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <Arduino.h>
#include "easyT6963.h"

easyT6963::easyT6963(){

}


/*-------------------------------------------------- CLEAR STUFF --------------------------------*/
//-------------------------------------------------------------------------------------------------
// Clears characters generator area of display RAM memory
//-------------------------------------------------------------------------------------------------
void easyT6963::clearCG(){
	unsigned int i = ((_memSize/2)-1) * 0x800;//30720
	setAddressPointer(i);
	for(i = 0; i < 2048; i++){
		writeDisplayData(0);
	}
	_gx = 0;
	_gy = 0;
}

/*
	Clear Display (and memory)
	fastMode(default) it's a bit faster
*/
void easyT6963::clearGraphic(uint8_t fastMode){
	unsigned int i;
	if (!fastMode){
		setAddressPointer(_graphicHome);
		for (i = 0; i < _graphicSize; i++){
			writeDisplayData(0x00);
		}
	} else {	
		setAddressPointer(_graphicHome);
		uint8_t h,j = 0;
		graphicGoTo(0,0);
		for (h = 0; h < _height; h++) {
			graphicGoTo(0,h); 
			for (i = 0; i < _maxCol+1; i++){     //(PIXELS PER LINE/FONT WIDTH * NUMBER OF LINES)
				writeDisplayData(0b00000000);
			}
			j = j + _maxCol;
		}
	}
	_gx = 0;
	_gy = 0;
}

/*
	Clears just Text
*/
void easyT6963::clearText(){
	unsigned int i;
	setAddressPointer(_textHome);
	for(i = 0; i < _textSize; i++){
		writeDisplayData(0);
	}
}

/*---------------------------------------------------- CHIP RELATED SPECIAL FUNCTIONS ----------------------*/


void easyT6963::clearDispMode(){
	sendCommand(T6963_DISPLAY_MODE);
}


void easyT6963::fastMode(bool mode){
	_fastMode = mode;
}

//-----------------------------------------------------------------------
//                       Set Display Mode   
//Example: LCD.setDispMode(true,true,false,false);    //true=ON	false=off 
//Parameter: ('GRAPHIC', 'TEXT', 'CURSOR', 'CURSOR BLINK') 
//Set blinking Cursor Example: 	LCD.setDispMode(true,true,true,true); //Mode all ON
//								LCD.setCursorPattern(8); // Cursor high
//								LCD.setCursorPointer(0,0); //Cursor Position	 
//-----------------------------------------------------------------------

uint8_t easyT6963::setDispMode(bool TEXT_SEL,bool GRAPHIC_SEL,bool CURSOR_SEL,bool CURSOR_BLINKING){
	uint8_t tmp = T6963_DISPLAY_MODE;
	if (GRAPHIC_SEL) tmp |= 0b1000; //T6963_GRAPHIC_DISPLAY_ON
	if (TEXT_SEL) tmp |= 0b0100; //T6963_TEXT_DISPLAY_ON
	if (CURSOR_SEL) tmp |= 0b0010; //T6963_CURSOR_DISPLAY_ON
	if (CURSOR_BLINKING) tmp |= 0b0001; //T6963_CURSOR_BLINK_ON
	sendCommand(tmp);
	return tmp;
}
//-----------------------------------------------------------------------
// Set Mode for Step 1 Text Manipulation and Show CGRam
//Example: LCD.setMode('0','I');           
//         
//Parameter: '0=Normal' 'X=Xor' 'A=And' 'T=Text Attribute Mode' , 'I=Intern CGram' , 'E=Extern CGram'
// Show Step 2 Example: (Set Text Attribute) to complete
//-----------------------------------------------------------------------

uint8_t easyT6963::setMode(uint8_t MODE_SEL, uint8_t CGRAM_SEL){
	uint8_t tmp = T6963_MODE_SET;
	switch(MODE_SEL){
		case OR: // 'OR' mode (default)  O, o
			tmp |= 0;
		break;
		case XOR: // mode  (mode == 'X' || mode == 'x' || mode== '^'){
			tmp |= 1;
		break;
		case AND: // mode  (mode == 'A' || mode == '&' || mode == 'a'){
			tmp |= 3;
		break;
		case TEXT: // attribute mode  ( mode == 'T'|| mode == 't'){
			tmp |= 4;
		break;
		default:
			tmp |= 0;
	}		
	if (CGRAM_SEL < 1){ //ICGRAM
		tmp |= 0;
	} else {			//ECGRAM  (cg == 'E'|| cg == 'e'){
		tmp |= 8;
	}
	sendCommand(tmp);
	return tmp;
}

void easyT6963::setAddressPointer(unsigned int address){
	sendData(address & 0xFF);
	sendData(address >> 8);
	sendCommand(T6963_SET_ADDRESS_POINTER);
}

unsigned int easyT6963::calculateAddress(uint8_t x, uint8_t y){
	unsigned int res = _graphicHome + (x / _fontWidth) + (_graphicArea * y);
	return res;
}

//-------------------------------------------------------------------------------------------------
// Writes display data and increment address pointer
//-------------------------------------------------------------------------------------------------
void easyT6963::writeDisplayData(byte x){
	sendData(x);
	sendCommand(T6963_DATA_WRITE_AND_INCREMENT);
}

void easyT6963::writeDataNon(byte x){
	sendData(x);
	sendCommand(T6963_DATA_WRITE_AND_NONVARIALBE);
}

void easyT6963::writeDisplayDataDec(byte x){
	sendData(x);
	sendCommand(T6963_DATA_WRITE_AND_DECREMENT);
}


/*-------------------------- Graphic Functions --------------------------------------*/
void easyT6963::setGraphicHome(unsigned int addr){
	addr == _textHome ? _graphicHome = _textHome + _textSize : _graphicHome = addr;
}
//-------------------------------------------------------------------------------------------------
// Set a single pixel at x,y (in pixels) black or white
//-------------------------------------------------------------------------------------------------
void easyT6963::drawPixel(uint8_t x, uint8_t y, bool color){
	if (x > _width && y > _height) return;
	byte tmp;
	color == 1 ? tmp = 0xF8 : tmp = 0xF0;//black or white 0b11111000 || 0b11110000
	graphicGoTo(x,y);
	tmp |= (_fontWidth - 1) - (x % _fontWidth); //LSB Direction Correction
	sendCommand(tmp);
}

/*
Version 2, a bit faster
* from Adafruit_GFX, based on Bresenham's algorithm
*/
void easyT6963::drawLine(uint8_t x0, uint8_t y0,uint8_t x1, uint8_t y1,bool color) {
	uint8_t dx, dy;
	uint8_t ystep;
	int err;
	uint8_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}
	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}
	dx = x1 - x0;
	dy = abs(y1 - y0);
	err = dx / 2;
	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}
	for (; x0<=x1; x0++) {
		if (steep) {
			drawPixel(y0, x0, color);
		} else {
			drawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

void easyT6963::drawFastVLine(uint8_t x, uint8_t y,uint8_t h, bool color) {
	drawLine(x, y, x, y+h-1, color);
}

void easyT6963::drawFastHLine(uint8_t x, uint8_t y,uint8_t w, bool color) {
	drawLine(x, y, x+w-1, y, color);
}

void easyT6963::drawTriangle(uint8_t x0, uint8_t y0,uint8_t x1, uint8_t y1,uint8_t x2, uint8_t y2, bool color) {
	drawLine(x0, y0, x1, y1, color);
	drawLine(x1, y1, x2, y2, color);
	drawLine(x2, y2, x0, y0, color);
}

void easyT6963::drawRect(uint8_t x, uint8_t y, uint8_t b, uint8_t a, bool color){
  uint8_t j; 
  // Draw vertical lines
  for (j = 0; j < a; j++) {
		drawPixel(x, y + j, color);
		drawPixel((x + b) - 1, y + j, color);
	}
  // Draw horizontal lines
  for (j = 0; j < b; j++)	{
		drawPixel(x + j, y, color);
		drawPixel(x + j, (y + a) - 1, color);
	}
}

/*
Example: lcd.drawFillRect (0,0,50,10,1);
Parameter: x1 and y1 top left Position  x2 and y2 bottom right Position
*/
void easyT6963::drawFillRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool color){
	uint8_t i;
	for (i = y1; i <= y2; i++){
		 drawLine(x1, i, x2, i, color);
	}
}


/*
Example: x,y = Start Position, Height in Pixel, Widht in Byte, pattern in byte(0b11111111)
*/
void easyT6963::drawFastRect(uint8_t x,uint8_t y,uint8_t height,uint8_t bytewidth,uint8_t pattern){ 
	int j = 0;
	uint8_t h,i;
	graphicGoTo(x,y);
	for (h = 0; h < height; h++) {
		graphicGoTo(x,y+h);
		for (i = 0; i < bytewidth; i++){ 
			writeDisplayData(pattern);
		}
		j = j + bytewidth;
	}
} 

/*
Example: lcd.createCircle(30, 30, 30, 1);
cx and cy mark the distance from the origin point.
*/
void easyT6963::drawCircle(uint8_t cx, uint8_t cy, uint8_t radius, bool color){
	int error = -radius;
	uint8_t x = radius;
	uint8_t y = 0;
	while (x >= y){
		plot8points(cx, cy, x, y, color);
		error += y;
		++y;
		error += y;
		if (error >= 0){
			--x;
			error -= x;
			error -= x;
		}
	}
}

/*
void easyT6963::drawHalfCircle(uint8_t x, uint8_t y, uint8_t radius,bool color){
  int xc = 0;
  int yc ;
  int p ;
  yc=radius;
  p = 3 - (radius << 1);
  while (xc <= yc)  {
    drawPixel(x + xc, y - yc, show);
    drawPixel(x - xc, y - yc, show);
    drawPixel(x + yc, y - xc, show);
    drawPixel(x - yc, y - xc, show);
    if (p < 0)
      p += (xc++ << 2) + 6;
    else
      p += ((xc++ - yc--)<<2) + 10;
  }
}
*/

void easyT6963::plot8points(uint8_t cx, uint8_t cy, uint8_t x, uint8_t y, bool color){
	plot4points(cx, cy, x, y, color);
	if (x != y) plot4points(cx, cy, y, x, color);
}
 
void easyT6963::plot4points(uint8_t cx, uint8_t cy, uint8_t x, uint8_t y, bool color){
	drawPixel(cx + x, cy + y, color);
	if (x != 0) drawPixel(cx - x, cy + y, color);
	if (y != 0) drawPixel(cx + x, cy - y, color);
	if (x != 0 && y != 0) drawPixel(cx - x, cy - y, color);
}

/*-------------------------------------------------- CURSOR STUFF --------------------------------------*/
/*
classic x,y pixel addressing
*/
void easyT6963::graphicGoTo(uint8_t x, uint8_t y){
	unsigned int address = calculateAddress(x,y);
	setAddressPointer(address);
}

/*
Emulated liquid crystal library set cursor
*/
void easyT6963::setCursor(uint8_t col, uint8_t row){
	unsigned int address = _textHome + col + (_graphicArea * row);
	setAddressPointer(address);
}


byte easyT6963::setCursorPattern(uint8_t b){
	byte tmp = T6963_CURSOR_PATTERN_SELECT;
	if (b > 7) b = 7;
	//b = constrain(b,0,7);
	tmp |= b;
	sendCommand(tmp);
	return tmp;
}

void easyT6963::setCursorPointer(uint8_t col,uint8_t row){
	if (col > _maxCol) col = _maxCol;
	if (row > _maxRow) row = _maxRow;
	sendData(col);
	sendData(row);
	sendCommand(0x21); //Cursor pointer Set
}


//-----------------------------------------------------------------------
//                               Show Picture(beta)
//Generate Picture File  by "BMP2C.exe datei.bmp 6" example for 6*8 Pixel
//http://www.holger-klabunde.de
//6 pixel per byte for T6963 and 6x8 font
//Horizontal pixel orientation for T6963
//Pixel orientation per Byte: D7..D0
// 
//Example: #include <demo.h>									//Filename
//		   LCD.drawBitmap( 0, 0, demobmp ,demoHEIGHT, demoBYTEWIDTH );; //Show Demo Picture

// Parameter :  x,y = Position, Picturename (not Filename see in Picturefile), Height in Pixel, Widht in Byte
 //HEIGHT = max 64;	//Height in pixels     BYTEWIDTH = max 40;	//Width in bytes by 6x8 Font
//-----------------------------------------------------------------------
 void easyT6963::drawBitmap(uint8_t x,uint8_t y, const uint8_t *PicName,uint8_t height,uint8_t bytewidth){ 
	int j = 0;
	uint8_t h,i,bitmap;
  	for (h = 0; h < height; h++){
		graphicGoTo(x,y+h);
		for (i = 0; i < bytewidth; i++) { 
			bitmap = pgm_read_byte_near(PicName+(i+j));
			writeDisplayData(bitmap);
		}
		j = j + bytewidth;
    }
}


void easyT6963::drawAnim(uint8_t x,uint8_t y, const uint8_t **string_table, int l,uint8_t height,uint8_t bytewidth){ //Position x, Position y, String of Images , Number of Images ,HEIGHT,BYTEWIDTH show bmp2c Picturefile
    uint8_t bitmap,a,h,i;
	int j;
	char* PicName;	 
	for (a = 0; a < l; a++) {
		PicName = (char*) pgm_read_word(&(string_table[a]));
		//PicName = pgm_read_word(&(string_table[a]));
		j = 0;
		for (h = 0; h < height-1; h++) {
			graphicGoTo(x,y+h);
			for (i = 0; i < bytewidth; i++){  
				bitmap = pgm_read_byte_near(PicName+(i+j));
				writeDisplayData(bitmap);
			}
			j = j + bytewidth;
		}
    }
}

/*------------------------------------------------------- TEXT STUFF --------------------------------------*/
void easyT6963::setTextHome(unsigned int addr){
	_textHome = addr;
}

uint8_t easyT6963::getTextCols(){
	return _graphicArea;
}
uint8_t easyT6963::getFontWidth(){
	return _fontWidth;
}
/*
void easyT6963::setGraphicFont(const struct FONT_DEF *strcut1) {
	_currentFont = strcut1;
}
*/
//-------------------------------------------------------------------------------------------------
// Writes single char pattern to character generator area of display RAM memory
//-------------------------------------------------------------------------------------------------
void easyT6963::defineCharacter(byte charCode, unsigned char * defChar){
	uint8_t i;
	unsigned int address = ((_memSize / 2)-1) * 0x800;
	setAddressPointer(address);
	for(i = 0; i < 8 ; i++){
		writeDisplayData(*(defChar + i));
	}
}

//-----------------------------------------------------------------------
//                               Set Text Attribute (Text only) (Step 2)
//Example: LCD.setMode('T','I');         
//         LCD.setTextAttrMode('0');     
//
//Parameter: '0=Normal display' '5=Reverse display' '3=Inhibit display' 
//			 '8=Blink of normal display' 'D=Blink of reverse display' 
//			 'B=Blink of inhibit display'
// Text only: 0=Normal display 5=Reverse display 3=Inhibit display 8=Blink of normal display D=Blink of reverse display B=Blink of inhibit display
//-----------------------------------------------------------------------

/*
NORMAL,REVERSE,INHIBIT,NBLINK,RBLINK,IBLINK
*/
uint8_t easyT6963::setTextAttrMode(uint8_t TEXT_ATTRIBUTE){   
	uint8_t tmp = 0;
	unsigned int i;
	switch(TEXT_ATTRIBUTE){
		case NORMAL:
			tmp |= 0;
		break;
		case REVERSE:
			tmp |= 0b0101;
		break;
		case INHIBIT:
			tmp |= 0b0011;
		break;
		case NBLINK:
			tmp |=0b1000;
		break;
		case RBLINK:
			tmp |= 0b1101;
		break;
		case IBLINK:
			tmp |= 0b1011;
		break;
		default:
			tmp |= 0;
	}
	setAddressPointer(_graphicHome);
	for(i = 0; i < _graphicSize; i++){
		writeDisplayData(tmp);    
	}
	return tmp;
}




void easyT6963::gPrint(uint8_t x,uint8_t y, const char *in, const struct FONT_DEF *strcut1,bool color){
	unsigned int offset;
	unsigned char by = 0, mask = 0;
	uint8_t i,j,height,width,NrBytes;
	unsigned char cmap;
	uint8_t allwidth = 0;
	//byte tmp;
	while ((cmap = *in++)) {
		cmap = pgm_read_byte(&strcut1->mapping_table[cmap]);
		width = strcut1->glyph_width;
		if (width == 0) width = pgm_read_byte(&strcut1->width_table[cmap]);
		offset = pgm_read_word(&strcut1->offset_table[cmap]);
		height = strcut1->glyph_height;
        NrBytes = ((width - 1) / 8) + 1;
		for (j = 0;j < (height * NrBytes); j+=NrBytes){// height
			for (i = 0;i < width; i++){//  width
			    if (i%8 == 0) {
					by = pgm_read_byte(&strcut1->glyph_table[offset + j + (i/8)]);
					mask = 0x80;
			    }
				_gx = x + (i + allwidth);
				_gy = y + (j / NrBytes);
				if (by & mask) {
					drawPixel(_gx,_gy, !color);
	 			} else {
	 				if (color) {
						drawPixel(_gx,_gy, color);
					} else {
						graphicGoTo(_gx,_gy);
					}
				}
	 			mask >>= 1;
			}//End i
		}// End j
		allwidth+=width;
	}// End K
} 

// --------------------------- SPECIAL write belongs to stream.h, need special attention -----------------------------------

size_t easyT6963::write(uint8_t value) {
	writeDisplayData(value - 32);
	return 1; //assume success  added for Arduino 1
}


