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


// Comment out if not using OLED
//#define OLED
Adafruit_SSD1305 display(128, 64, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

/////////////////////////////////////
// Utilities: 
#ifdef OLED
#define DP_PRINTLN(x) { \
  display.println(F(x)); \  //use F(X) to save the string to Flash
  display.display()
}
#define DP_PRINT(x) { \
  display.print(F(x)); \  //use F(X) to save the string to Flash
  display.display()
}
#else
#define DP_PRINTLN(x) Serial.println(F(x))   //use F(X) to save the string to Flash
#define DP_PRINT(x) Serial.print(F(x))       //use F(X) to save the string to Flash
#endif

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

inputs input = (inputs)0;
int current_state = START_SCREEN;
int next_state = 0;
int score = 0;
bool over = false;

////////////////////////////////////
//          MICROPHONE            //
////////////////////////////////////
#define MIC_IN A5
#define BG_SCALE 1.3

bool get_voice_input();
int background_sound = 0;

////////////////////////////////////

void reset_display(){
  #ifdef OLED
  display.clearDisplay(); //use this to clear display

  //clear display resets all settings, configure text:
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0); //cursor start in top left
  #endif
}

void setup() {
  Serial.begin(9600);
  while (! Serial) delay(100);

  #ifdef OLED
  if ( ! display.begin(0x3C) ) {
     while (1) yield();
  }
  #endif

  reset_display();

  background_sound = 1.30*analogRead(MIC_IN);

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
  reset_display();
  
  DP_PRINTLN("Disarm-it!");
  DP_PRINTLN("Scan finger to start...");


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

    next_state = HANDLE_INPUT;
  }
  else{
    next_state = GAME_OVER;
  }

}


void handle_input(){
  reset_display();
  
  DP_PRINTLN("handling input");
  delay(100);
  
  switch(input){
    case FINGERPRINT_SCANNER:
      //call fingerprint scanner fn
      break;
    case KEYPAD:
      //call keypad function
      break;
    case MICROPHONE:
      over = get_voice_input();
      break;
  }
  next_state = WAIT_AND_SELECT_INPUT;
}

bool get_voice_input(){
  DP_PRINTLN("Voice recognition");
  DP_PRINTLN("Speak into the microphone");
  bool g_over = true;
  
  while(g_over){
    int current = analogRead(MIC_IN);
    g_over = current<=background_sound;
  }
  
  return g_over;
}

void game_over(){
  
}
