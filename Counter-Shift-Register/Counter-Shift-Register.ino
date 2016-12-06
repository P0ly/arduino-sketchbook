int dataPin = 2;
int shiftPin = 4;
int storePin = 3;
byte counter = 0;

void shift(int dataPin, int shiftPin, int value) {
  digitalWrite(shiftPin, LOW);
  digitalWrite(dataPin, value);
  digitalWrite(shiftPin, HIGH);
}

void setup() {
  
  pinMode(dataPin,OUTPUT);
  pinMode(shiftPin, OUTPUT);
  pinMode(storePin, OUTPUT);

}

void loop() {

  counter++;

  //for(int i = 0; i < 8; i++) {
    
  //    digitalWrite(storePin, LOW);
  //    shift(dataPin, shiftPin, counter >> i & 1);
  //    digitalWrite(storePin, HIGH);
      
  //}

  for(int i = 7; i >= 0; i--) {
    
      digitalWrite(storePin, LOW);
      shift(dataPin, shiftPin, counter >> i & 1);
      digitalWrite(storePin, HIGH);
      
  }
  
  delay(1000);

}
