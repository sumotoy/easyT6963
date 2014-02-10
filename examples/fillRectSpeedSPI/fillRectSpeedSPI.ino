#include <SPI.h>
#include <mcp23s17.h>   // needed!
#include <T6963_SPI.h>
//#include "fonts/Times_New_Roman__14.h"
//#include "gfxdata.h"

#if (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__))      //--- Arduino Mega ---
T6963_SPI lcd(53,0x20,1);//53,0x20
#else	
T6963_SPI lcd(10,0x20,1);//10,0x20
#endif


void setup() {
  //Serial.begin(115200);
  lcd.begin(240,128,T6963_6x8DOTS,32);//240,128,T6963_6x8DOTS,32
  lcd.setBacklight(1);

}

void loop() {
  timeFPS();
}

boolean _mode = true;

void timeFPS()
{
  if (_mode){
     _mode = false;
    lcd.fastMode(_mode);
  } else {
     _mode = true;
    lcd.fastMode(_mode);
  }
  unsigned long stime, etime;
  unsigned long sp;
  lcd.clearText();
  lcd.setCursor(0,0);
  lcd.print("       Speed Demo ");
  if (_mode){
      lcd.print("FastMode enabled");
  } else {
      lcd.print("Normal Mode");
  }
  lcd.drawRect(4,10,112,87,1);//x,y=Start Position,Height,Width,Color
  stime = micros();
  //(x, y, height, bytewidth, pattern) // x,y=Start Position,Height,Width in Byte (6x8Font=6Bits 8x8Font=8Bits),Fill pattern in byte Example:0b10101010 
  lcd.drawFastRect(10,12,83,18,0b111111);                  
  etime = micros(); 
  sp = (etime-stime)/60;
  lcd.setCursor(1,14);
  lcd.print("write Byte:");
  lcd.print(sp,DEC);
  lcd.drawRect(124,10,112,87,1);
  stime = micros();
  lcd.drawFillRect(126,12,233,94,1);//X,Y(start),X1,Y1(end),Color
  etime = micros(); 
  sp = (etime-stime)/60;
  lcd.setCursor(19,14);
  lcd.print("write Pixel:");
  lcd.print(sp,DEC);
  delay(100);
  lcd.drawFastRect(10,12,83,18,0b00000000);//Clear Box byte
  lcd.drawFastRect(126,12,83,18,0b00000000);//Clear Box Pixel
  lcd.clearGraphic();
}
