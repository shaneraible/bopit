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


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (! Serial) delay(100);
  Serial.println("SSD1305 OLED test");
  
  if ( ! display.begin(0x3C) ) {
     Serial.println("Unable to initialize OLED");
     while (1) yield();
  }

  // init done
  display.display(); // show splashscreen
  delay(1000);
//  display.clearDisplay();   // clears the screen and buffer
}

void loop() {
  // put your main code here, to run repeatedly:
  

}
