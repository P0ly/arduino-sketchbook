
/* CONFIG */
int leds[] = {4,5,6,7};
int buttonPin = 2;
int idleAniDelay = 250;
int idleAni[] = {
  0,0,0,0,
  1,0,0,0,
  1,1,0,0,
  1,1,1,0,
  1,1,1,1,
  0,0,0,0,
  0,0,0,1,
  0,0,1,1,
  0,1,1,1,
  1,1,1,1
};
int pushAniDelay = 750;
int pushAni[] = {
  0,0,0,0,
  1,1,1,1,
  0,0,0,0,
  1,1,1,1,
  0,0,0,0,
  1,1,1,1
};

/* GLOBAL VAR */
int numOfLeds = sizeof(leds)/sizeof(int);
int idleAniSize = sizeof(idleAni)/sizeof(int);
int pushAniSize = sizeof(pushAni)/sizeof(int);
int button = 0;
int i = 0;
int i2 = 0;
int state = 0;
unsigned long preTime = 0;
unsigned long curTime = 0;

void setup() {

  for(int i = 0; i < numOfLeds; i++) {
    pinMode(leds[i], OUTPUT); 
  }
  pinMode(buttonPin, INPUT);
  
}

void loop() {
  
  button = digitalRead(buttonPin);
  if(button == 1 && state != 1) state = 1;
  
  curTime = millis();
  if(state == 0) {
    
    if(curTime - preTime > idleAniDelay) {
      
      preTime = curTime;
      
      for(int l = 0; l < numOfLeds; l++) {
        digitalWrite(leds[l], idleAni[i+l]);
      }
      i+=numOfLeds;
      if(i > idleAniSize-numOfLeds) i = 0;
    
    }
    
  } else {
    
    if(curTime - preTime > pushAniDelay) {
       
        preTime = curTime;
        if(i2 > pushAniSize-numOfLeds) {
          
          i2 = 0;
          state = 0;
          
        } else {
          
          for(int l = 0; l < numOfLeds; l++) {
            digitalWrite(leds[l], pushAni[i2+l]);        
          }
          i2+=numOfLeds;
          
        }
    
    }
  
  }
  
}
