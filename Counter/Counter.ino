/* CONFIG */
// Max Leds = 16 = 2 byte = size of int(on Arduino UNO)
int leds[] = {2,3,4,5,6,7,8,9};
// time of delay in ms
const int tick = 1000;

/* GLOBAL VAR */
byte numOfLeds = sizeof(leds)/sizeof(int);
unsigned int counter = 0;

void setup() {

 for(int i = 0; i < numOfLeds; i++) {
    pinMode(leds[i], OUTPUT);
 }

 // Serial.begin(9600);

}

void loop() {

  counter++;
  if(counter > pow(2,numOfLeds)) counter = 0;

  for(int i = 0; i < numOfLeds; i++) {
     digitalWrite(leds[numOfLeds-i-1], counter >> i & 1);
    //  Serial.print(counter >> i & 1);
  }
  // Serial.println();
  // Serial.println(counter);

  delay(tick);

}
