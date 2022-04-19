#include <Talkie.h>
#include "Vocab_Soundbites.h"
#include <TalkieUtils.h>
#include <Vocab_US_Large.h>
#include <Vocab_Special.h>

///////////////////////////////////////////////
// DEFINITIONS FOR OLED DRIVER
///////////////////////////////////////////////
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1305.h>

#define OLED_CLK A0 
#define OLED_MOSI 10
#define SSD1305_128_64
#define OLED_CS 9
#define OLED_DC A1
#define OLED_RESET 13

Adafruit_SSD1305 display(128, 64, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
///////////////////////////////////////////////
// FINGERPRINT SENSOR
///////////////////////////////////////////////

// Utilities: 

// define print functions for use w/ screen
#define DP_PRINTLN(x) display.println(F(x))   //use F(X) to save the string to Flash
#define DP_PRINT(x) display.print(F(x))       //use F(X) to save the string to Flash


Talkie voice;

void setup() {
  Serial.begin(9600);
  while (! Serial) delay(100);
  display.println("SSD1305 OLED test");

  display.setRotation(2);
  
  if ( ! display.begin(0x3C) ) {
     display.println("Unable to initialize OLED");
     while (1) yield();
  }

  // init done
  display.display(); // show splashscreen
  delay(4);
  display.clearDisplay();   // clears the screen and buffer

  tone(3, 400, 100);

}

void loop() {

  DP_PRINTLN("This is how you print a new line");
  display.display();  //call this to flash to the display
  delay(300);         //delay 300ms (clocck no good)
  DP_PRINTLN("another line");
  display.display();
  delay(300);

  display.clearDisplay(); //use this to clear display

  //clear display resets all settings, configure text:
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0); //cursor start in top left

}
