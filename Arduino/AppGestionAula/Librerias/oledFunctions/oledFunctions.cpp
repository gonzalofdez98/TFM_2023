#include "oledFunctions.h"

void oledDisplayImage(int imagen, Adafruit_SSD1306 &oled){
  oled.clearDisplay();
  delay(100);
  // display bitmap
  if(imagen == 0){
    oled.drawBitmap(0, 0, aceptado, 128, 64, WHITE);
  }
  else if(imagen == 1){
    oled.drawBitmap(0, 0, denegado, 128, 64, WHITE);
  }
  else if(imagen == 2){
    oled.drawBitmap(0, 0, advertencia, 128, 64, WHITE);
  }
    
  oled.display();
  delay(1000);
}

void oledDisplayMsg(String msg1, String msg2, String msg3, Adafruit_SSD1306 &oled){
  oled.clearDisplay(); // clear display
  delay(100);
  oled.setTextSize(1);         
  oled.setTextColor(WHITE);    
  oled.setCursor(0, 0);       
  oled.println(msg1);
  if(msg2.length() > 0){
    oled.setCursor(0, 20); 
    oled.println(msg2);
  }
  if(msg3.length() > 0){
    oled.setCursor(0, 40); 
    oled.println(msg3);
  }
  oled.display();
  delay(100);
  oled.clearDisplay(); 
}