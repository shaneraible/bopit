#define newLED A3
#define button 9

void setup() {
  // initialize the LED pin as an output:
  pinMode(newLED, OUTPUT);
  pinMode(button, INPUT);
}

void loop() {
    int pushButton = digitalRead(button);
      
    if(pushButton == HIGH){
      digitalWrite(newLED, HIGH);
      delay(1000);
      digitalWrite(newLED, LOW);
      delay(1000); 
    }
 
}
