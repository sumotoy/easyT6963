#include <SPI.h>
#include <mcp23s17.h>   // needed!
#include <T6963_SPI.h>
#include "fonts/Dubsteptrix__12.h"
#include "fonts/Imagine_FontFixed__6.h"
/*
monoMMM_Fixed__6
 Square_HeadCon_Fixed__9
 M39_SQUAREFUTURE_Fixed__5
 Indieutka_PixelFixed__12
 Imagine_FontFixed__15
 Imagine_FontFixed__6
 genown_one_fixed__5
 DubsteptrixFixed__8
 Dubsteptrix__12
 BeatboxFixed__8
 Beatbox__9
 Identification_Mono__8
 Square_Pixel7__14
 SnareDrum_One_NBP__12
 Redensek__9
 Pixel_Millennium__12b
 Pixel_Millennium__12
 Megaton__12
 Megaton__9
 Imagine_Font__12b
 Imagine_Font__12
 Imagine_Font__9
 Homespun_TT_BRK__9
 */
#if (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__))      //--- Arduino Mega ---
T6963_SPI lcd(53,0x20,1);//53,0x20
#else	
T6963_SPI lcd(10,0x20,1);//10,0x20
#endif

void printIt(uint8_t x,uint8_t y,char* txt,boolean color){
  lcd.gPrint(x,y,txt,&Dubsteptrix__12,color);
}

void printIt2(uint8_t x,uint8_t y,char* txt,boolean color){
  lcd.gPrint(x,y,txt,&Imagine_FontFixed__6,color);
}

boolean _fastmode;
uint8_t _line1;
uint8_t _line2;

void setup() {
  //Serial.begin(115200);
  randomSeed(analogRead(0));
  lcd.begin(240,128,T6963_6x8DOTS,32);//240,128,T6963_6x8DOTS,32
  lcd.setBacklight(1);
  lcd.setMode(NORMAL);//Switch to Normal Mode
}

void loop() {
  _fastmode = random(0,2);
  lcd.fastMode(_fastmode);

  _line1 = random(0,2);
  _line2 = random(0,2);
  if (_fastmode){
    lcd.setCursor(0,10);
    lcd.print("Fast Mode Enabled...");
  } 
  else {
    lcd.setCursor(0,10);
    lcd.print("Slow secure mode ...");
  }  
  if (_line1 == 0){
    printIt(3, 3, "T6963C LCD", 0);
  } 
  else {
    printIt2(3, 3, "T6963C LCD", 0);
  }
  if (_line2 == 0){
    printIt(3, 25, "ext. Fonts", 0);
  } 
  else {
    printIt2(3, 25, "ext. Fonts", 0);
  }

  delay(2000);
  lcd.clearGraphic();

}
