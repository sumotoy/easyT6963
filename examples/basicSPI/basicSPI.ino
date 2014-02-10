#include <SPI.h>
#include <mcp23s17.h>   // needed!
#include <T6963_SPI.h>

#if (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__))      //--- Arduino Mega ---
T6963_SPI lcd(53,0x20,1);//53,0x20
#else	
T6963_SPI lcd(10,0x20,1);//53,0x20
#endif


void setup() {
  //Serial.begin(115200);
  //240x128 display
  lcd.begin(240,128,T6963_6x8DOTS,32);//240,128,T6963_6x8DOTS,32
  lcd.fastMode(true);//try also false
  lcd.setBacklight(1);
  lcd.drawCircle(30,30,30,1);
  lcd.drawFillRect (70,70,100,100,1);

}


void loop() {
  uint8_t w = random(0,239);
  uint8_t h = random(0,127);
  lcd.drawPixel(w,h,1);
  delay(100);
  lcd.drawPixel(w,h,0);
  delay(100);
}
