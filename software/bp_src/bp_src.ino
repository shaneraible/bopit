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

#include <Adafruit_Fingerprint.h>
#define mySerial Serial
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Utilities: 

// define print functions for use w/ screen
#define DP_PRINTLN(x) display.println(F(x))   //use F(X) to save the string to Flash
#define DP_PRINT(x) display.print(F(x))       //use F(X) to save the string to Flash


///////////////////////////////////
//        GAME SETTINGS          //
///////////////////////////////////
#define START_DELAY 2000
#define DELAY_STEP  100

int delay_ms = START_DELAY;

///////////////////////////////////
//    STATES FOR STATE MACHINE   //
///////////////////////////////////

typedef void (*State) ();

void start_screen();
void handle_input();
void wait_and_select_input();
void game_over();

enum states {
  START_SCREEN,
  HANDLE_INPUT,
  WAIT_AND_SELECT_INPUT,
  GAME_OVER
};

#define NUM_INPUTS 3
enum inputs {
  FINGERPRINT_SCANNER,
  KEYPAD,
  MICROPHONE
};


State states[] = {start_screen, handle_input, wait_and_select_input, game_over};

inputs input = 0;
int current_state = 0;
int next_state = 0;
int score = 0;
bool over = false;

////////////////////////////////////

void reset_display(){
  display.clearDisplay(); //use this to clear display

  //clear display resets all settings, configure text:
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0); //cursor start in top left
}

void setup() {
  Serial.begin(9600);
  while (! Serial) delay(100);
  
  if ( ! display.begin(0x3C) ) {
     while (1) yield();
  }


  // set the data rate for the sensor serial port
  finger.begin(57600);
//  delay(5);
  finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
  display.clearDisplay();

//  // set the data rate for the sensor serial port
//  if (finger.verifyPassword()) {
//    DP_PRINTLN("Found fingerprint sensor!");
//    display.display();
//    delay(300);
//    display.clearDisplay();
//  } else {
//    DP_PRINTLN("Did not find fingerprint sensor :(");
//    display.display();
//    delay(300);
//    display.clearDisplay();
//    while (!finger.verifyPassword()) { delay(1); }
//    
//  }

}

void loop() {
  current_state = next_state;
  states[current_state]();
}


bool check_start_input(){
  delay(10);
  return true;
}
void start_screen() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  
  DP_PRINTLN("Disarm-it!");
  DP_PRINTLN("Scan finger to start...");
  display.display();

  while(!check_start_input()){}

  next_state = WAIT_AND_SELECT_INPUT;
}

void wait_and_select_input(){
  reset_display();
  
  if(!over && score < 100){
    input = rand()%NUM_INPUTS;
    delay(delay_ms);
    switch(input){
      case FINGERPRINT_SCANNER:
        DP_PRINTLN("Scan it!");
        break;
      case KEYPAD:
        DP_PRINTLN("Password it!");
        break;
      case MICROPHONE:
        DP_PRINTLN("Speak it!");
        break;
    }

    display.display();
    next_state = handle_input();
  }
  else{
    next_state = GAME_OVER;
  }

}


void handle_input(){
  reset_display();
  DP_PRINTLN("handling input");
  display.display();
  delay(100);
  
  switch(input){
    case FINGERPRINT_SCANNER:
      //call fingerprint scanner fn
    case KEYPAD:
      //call keypad function
    case MICROPHONE:
      //call microphone function
      break;
  }
}

void game_over(){
  
}


//  DP_PRINTLN("This is how you print a new line");
//  display.display();  //call this to flash to the display
//  delay(300);         //delay 300ms (clocck no good)
//  DP_PRINTLN("another line");
//  display.display();
//  delay(300);
//
//  display.clearDisplay(); //use this to clear display
//
//  //clear display resets all settings, configure text:
//  display.setTextSize(1);
//  display.setTextColor(WHITE);
//  display.setCursor(0,0); //cursor start in top left
