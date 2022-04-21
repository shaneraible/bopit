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

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h);

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
String input_password = "FFFF";
bool getcode = false;

///////////////////////////////////
//        GAME SETTINGS          //
///////////////////////////////////
#define START_DELAY 2000
#define DELAY_STEP  100

#define FINGERPRINT_TONE 600
#define KEYPAD_TONE 200
#define unlock_TONE 400
#define TONE_DURATION 100
#define TONE_PIN 3

#define MAX_SCORE 10
#define START_DT 8000
#define MIN_DT 3000

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

long timeout      = 0;
int time_step     = START_DT;
int time_sub      = (START_DT-MIN_DT)/MAX_SCORE;
  

////////////////////////////////////
//          MICROPHONE            //
////////////////////////////////////
#define MIC_IN A5
#define BG_SCALE 1.2

bool get_unlock_input();
int background_sound = 0;
int prev_pos = 0;

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
  DISPLAY();
  
  current_touchstate = digitalRead(touchsensor);
  background_sound = BG_SCALE*analogRead(MIC_IN);
  
  input_password.reserve(32); // maximum input characters is 33, change if needed

  pinMode(touchsensor, INPUT);

}

void loop() {
  current_state = next_state;
  states[current_state]();
}


void start_screen() {
  reset_display();
  current_touchstate == digitalRead(touchsensor);
  background_sound = BG_SCALE*analogRead(MIC_IN);
  
  DP_PRINTLN("Disarm-it!");
  DP_PRINTLN("");
  DP_PRINTLN("Scan finger to start");
  DP_PRINTLN("");
  DP_PRINT("Your passcode is ");
  display.println(password);
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
  prev_pos = analogRead(MIC_IN);
  
  if(!over && score < MAX_SCORE){
    int i = input;
    input = rand()%NUM_INPUTS;

    if(input == i){
      input = rand()%NUM_INPUTS;
    }

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
        DP_PRINTLN("Unlock it!");
        tone(TONE_PIN, unlock_TONE, TONE_DURATION);
        DISPLAY();
        delay(1500);
        break;
    }
    DISPLAY();

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
      over = get_unlock_input();
      break;
  } 

   if(!over) score++;
  if (!over && score<MAX_SCORE) tone(TONE_PIN, 1000, 500);
  //delay until next state
  while(score<MAX_SCORE && !over && millis() < timeout){
    reset_display();
    DP_PRINTLN("");
    DP_PRINTLN("Level disarmed!");
    DP_PRINT("Next level in ");
    current_touchstate = digitalRead(touchsensor);
    long s = (timeout - millis())/1000;
    display.print(s);
    DP_PRINTLN("s");
    DP_PRINT("Your score is ");
    display.println(score);
    DP_PRINT("You have ");
    display.print (time_step);
    DP_PRINTLN("ms next round");
    DISPLAY();
  }
  
  time_step = time_step - time_sub;


  next_state = WAIT_AND_SELECT_INPUT;
}

bool get_unlock_input(){

  DP_PRINTLN("Twist the key!");
  DISPLAY();
  
  bool g_over = true;
  int current = 0;
  
  delay(50);
  
  while(g_over && millis() < timeout){
    
    current = analogRead(MIC_IN);
    g_over = abs(current-prev_pos)<500 ;
  }
  return g_over;
}

void game_over(){
  if(score < MAX_SCORE){
    tone(TONE_PIN, 50, 1000);
    testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);
    DP_PRINTLN("GAME OVER!");
    DP_PRINT("YOUR SCORE WAS: ");
  }
  else{
    tone(TONE_PIN, 500, 1000);
    testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);
    DP_PRINTLN("Winner!");
    DP_PRINTLN("");
    DP_PRINT("Score: ");
    display.println(score);
  }
  
  display.print(score);
  DISPLAY();

  score = 0;
  over = false;

  delay(10000);
  current_touchstate = digitalRead(touchsensor);
  next_state = START_SCREEN;
  time_step = START_DT;
}

bool keypad_logic(){
  input_password = ""; // clear input password
  DP_PRINTLN("Enter password below:");
  DISPLAY();
  
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
          DP_PRINTLN("Correct!");
          DISPLAY();
          return false;
        } else {
          DP_PRINTLN("");
          DP_PRINT("You entered: ");
          display.println(input_password);
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
  current_touchstate = digitalRead(touchsensor);
  DP_PRINTLN("SCAN FINGER");
  DISPLAY();
  delay(50);
  while(millis() < timeout){
      if(current_touchstate != digitalRead(touchsensor))
      {
        current_touchstate = digitalRead(touchsensor);
        return false;
      }
  }

  return true;
}

void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  uint8_t icons[NUMFLAKES][3];
  randomSeed(666);     // whatever seed
 
  // initialize
  for (uint8_t f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS] = random(display.width());
    icons[f][YPOS] = 0;
    icons[f][DELTAY] = random(5) + 1;
    
    Serial.print("x: ");
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(" y: ");
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(" dy: ");
    Serial.println(icons[f][DELTAY], DEC);
  }
  int count = 0;
  while (count<24) {
    // draw each icon
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], logo16_glcd_bmp, w, h, WHITE);
    }
    display.display();
    delay(20);
    
    // then erase it + move it
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS],  logo16_glcd_bmp, w, h, BLACK);
      // move it
      icons[f][YPOS] += icons[f][DELTAY];
      // if its gone, reinit
      if (icons[f][YPOS] > display.height()) {
        icons[f][XPOS] = random(display.width());
        icons[f][YPOS] = 0;
        icons[f][DELTAY] = random(5) + 1;
      }
    }
    count++;
   }
}
