/* CONFIG */
int leds[] = {4,5,6,7}; 
int buttons[] = {2,3};

/* GLOBAL VAR */
byte numOfLeds = sizeof(leds)/sizeof(int);
byte numOfButtons = sizeof(buttons)/sizeof(int);
unsigned long preTime = 0;
unsigned long curTime = 0;
byte counter = 0;
boolean state = 0;

void setup() {
 
 for(int i = 0; i < numOfLeds; i++) {
    pinMode(leds[i], OUTPUT); 
 }
 
 for(int i = 0; i < numOfButtons; i++) {
    pinMode(buttons[i], INPUT); 
 }
 
 //Serial.begin(9600);
  
}

void displayNum(byte num) {
  
  for(int i = 0; i < numOfLeds; i++) {
     digitalWrite(leds[numOfLeds-i-1], num >> i & 1);
     //Serial.print(num >> i & 1);
  }
  //Serial.println();
  //Serial.println(num);
  
}

void loop() {
  
  if(state == 0) {
    
    state = 1;
    
   if(digitalRead(buttons[0]) == 1) {
      
      counter++;
      if(counter >= pow(2,numOfLeds)) counter = 0;
      displayNum(counter);
      
    } else if(digitalRead(buttons[1]) == 1) {
     
      counter--;
      if(counter < 0) counter = pow(2,numOfLeds)-1;
      displayNum(counter);
      
    }
    
  } else if(digitalRead(buttons[0]) == 1 && digitalRead(buttons[1]) == 1) {
  
    counter = 0;
    displayNum(counter);
    
  } else {
  
    if(digitalRead(buttons[0]) == 0 && digitalRead(buttons[1]) == 0) state = 0;
    
  }
  
    
}
