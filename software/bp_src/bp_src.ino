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
#define OLED
Adafruit_SSD1305 display(128, 64, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

/////////////////////////////////////
// Utilities: 
#ifdef OLED
#define DP_PRINTLN(x) display.println(F(x))  //use F(X) to save the string to Flash
#define DP_PRINT(x) display.print(F(x)) //use F(X) to save the string to Flash
#define DISPLAY() display.display()
#else
#define DP_PRINTLN(x) Serial.println(F(x))   //use F(X) to save the string to Flash
#define DP_PRINT(x) Serial.print(F(x))       //use F(X) to save the string to Flash
#define DISPLAY()
#endif

///////////////////////////////////////////////
// KEYPAD SENSOR
///////////////////////////////////////////////
//https://arduinogetstarted.com/tutorials/arduino-keypad
#include <Keypad.h>

const int ROW_NUM = 4; //four rows
const int COLUMN_NUM = 3; //three columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte pin_rows[ROW_NUM] = {A3, 7, 6, 4}; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {A4, A2, 5}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
String randompasscode();
String password = randompasscode(); // change your password here
String input_password;
bool getcode = false;

///////////////////////////////////
//        GAME SETTINGS          //
///////////////////////////////////
#define START_DELAY 2000
#define DELAY_STEP  100

#define FINGERPRINT_TONE 600
#define KEYPAD_TONE 200
#define VOICE_TONE 400
#define TONE_DURATION 100
#define TONE_PIN 3

///////////////////////////////////
//    STATES FOR STATE MACHINE   //
///////////////////////////////////

typedef void (*State) ();

void start_screen();
void handle_input();
void wait_and_select_input();
void game_over();
bool keypad_logic();

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

inputs input      = (inputs)0;
int current_state = START_SCREEN;
int next_state    = 0;
int score         = 0;
bool over         = false;

long timeout       = 0;
int time_step     = 10000;

////////////////////////////////////
//          MICROPHONE            //
////////////////////////////////////
#define MIC_IN A5
#define BG_SCALE 1.2

bool get_voice_input();
int background_sound = 0;

////////////////////////////////////

////////////////////////////////////
//          TOUCH SENSOR          //
////////////////////////////////////
#define touchsensor 2
int current_touchstate = LOW;


void reset_display(){
  #ifdef OLED
  display.clearDisplay(); //use this to clear display

  //clear display resets all settings, configure text:
  display.setRotation(2);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0); //cursor start in top left
  #endif
}

void setup() {
  #ifndef OLED
  Serial.begin(9600);
  while (! Serial) delay(100);
  #endif

  #ifdef OLED
  if ( ! display.begin(0x3C) ) {
     while (1) yield();
  }
  #endif

  reset_display();
  
  current_touchstate = digitalRead(touchsensor);
  background_sound = BG_SCALE*analogRead(MIC_IN);
  
  display.println(analogRead(MIC_IN));
  display.println(background_sound);
  display.display();
  delay(500);
  input_password.reserve(32); // maximum input characters is 33, change if needed

  pinMode(touchsensor, INPUT);
}

void loop() {
  current_state = next_state;
  states[current_state]();
}


bool check_start_input(){
  delay(1);
  return true;
}


void start_screen() {
  reset_display();
  current_touchstate == digitalRead(touchsensor);
  background_sound = BG_SCALE*analogRead(MIC_IN);
  
  DP_PRINTLN("Disarm-it!");
  DP_PRINTLN("Scan finger to start...");
  DP_PRINT("Passcode is ");
  display.print(password);
  DP_PRINTLN(" for the entire game.");
  DISPLAY();

  while(current_touchstate == digitalRead(touchsensor)){};
  
  reset_display();
  for (uint8_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.display();
  }

  next_state = WAIT_AND_SELECT_INPUT;
}

void wait_and_select_input(){
  reset_display();
  current_touchstate == digitalRead(touchsensor);
  
  if(!over && score < 100){
    input = rand()%NUM_INPUTS;

    switch(input){
      case FINGERPRINT_SCANNER:
        DP_PRINTLN("Scan it!");
        tone(TONE_PIN, FINGERPRINT_TONE, TONE_DURATION);
        break;
      case KEYPAD:
        DP_PRINTLN("Password it!");
        tone(TONE_PIN, KEYPAD_TONE, TONE_DURATION);
        break;
      case MICROPHONE:
        DP_PRINTLN("Speak it!");
        tone(TONE_PIN, VOICE_TONE, TONE_DURATION);
        break;
    }
    DISPLAY();
    delay(1500);
    
    timeout = millis() + time_step;
    next_state = HANDLE_INPUT;
  }
  else{
    next_state = GAME_OVER;
  }
}


void handle_input(){
  switch(input){
    case FINGERPRINT_SCANNER:
      over = fingerprint_scanner_input();
      break;
    case KEYPAD:
      over = keypad_logic();
      break;
    case MICROPHONE:
      over = get_voice_input();
      break;
  }

   if(!over) score++;
  
  //delay until next state
  while(!over && millis() < timeout){
    reset_display();
    DP_PRINTLN("Level disarmed!");
    DP_PRINT("Next level in ");
    long s = (timeout - millis())/1000;
    display.print(s);
    DP_PRINTLN("s");
    DP_PRINT("Your score is ");
    display.println(score);
    DISPLAY();
    
  };
  time_step = time_step - 25;


  next_state = WAIT_AND_SELECT_INPUT;
}

bool get_voice_input(){
  DP_PRINTLN("Voice recognition...");
  DP_PRINTLN("Speak into the microphone");
  DISPLAY();
  
  bool g_over = true;
  int current = 0;
  
  delay(50);
  
  while(g_over && millis() < timeout){
    display.print(millis());

    current = analogRead(MIC_IN);
    display.print(current);
    g_over = current<=background_sound;
  }
  return g_over;
}

void game_over(){
  tone(TONE_PIN, 100, 1000);

  
  DP_PRINTLN("GAME OVER!");
  DP_PRINT("YOUR SCORE IS: ");
  display.print(score);
  DISPLAY();

  delay(10000);
  next_state = START_SCREEN;
}

bool keypad_logic(){
  input_password = ""; // clear input password
  DP_PRINTLN("Enter password below:");
  
  while(millis() < timeout){
    char key = keypad.getKey();
    if (key){
      display.print(key);

      if(key == '*') {
        input_password = ""; // clear input password
        reset_display();
        DP_PRINTLN("Enter password below:");
        DISPLAY();
      } else if(key == '#') {
        if(password == input_password) {
          DP_PRINTLN("");
          DP_PRINTLN("password is correct");
          DISPLAY();
          return false;
        } else {
          DP_PRINTLN("password is incorrect, try again");
          DISPLAY();
        }
  
        input_password = ""; // clear input password
      } else {
        input_password += key; // append new character to input password string
      }
      DISPLAY();
    }
  }

  return true;
}

String randompasscode(){
  String pass = "";
  
  for(int i=0; i<4; i++)
  {
    int digit = (rand()%9)+1;
    String digitchar = String(digit);
    pass += digitchar;
  }

  return pass;
}

bool fingerprint_scanner_input(){
  while(millis() < timeout){
      if(current_touchstate != digitalRead(touchsensor))
      {
        current_touchstate = digitalRead(touchsensor);
        return false;
      }
  }

  return true;
}
