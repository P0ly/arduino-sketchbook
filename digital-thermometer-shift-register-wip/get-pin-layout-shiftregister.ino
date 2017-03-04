const int data_pin = 2;
const int shift_pin = 3;
const int store_pin = 4;
const int trans_pin1 = 5;
const int trans_pin2 = 6;
const int temp_pin = A0;
int led = 0;

int test[][8] = {
  {1,0,0,0,0,0,0,0}, // Q0
  {0,1,0,0,0,0,0,0}, // Q1
  {0,0,1,0,0,0,0,0}, // Q2
  {0,0,0,1,0,0,0,0}, // Q3
  {0,0,0,0,1,0,0,0}, // Q4
  {0,0,0,0,0,1,0,0}, // Q5
  {0,0,0,0,0,0,1,0}, // Q6
  {0,0,0,0,0,0,0,1}, // Q7
  {1,1,1,1,1,1,1,1}  // let all leds light up on index 8
};

void setup() {

  pinMode(data_pin,OUTPUT);
  pinMode(shift_pin, OUTPUT);
  pinMode(store_pin, OUTPUT);
  pinMode(trans_pin1, OUTPUT);
  pinMode(trans_pin2, OUTPUT);
  pinMode(test_led, OUTPUT);

  digitalWrite(data_pin, LOW);
  digitalWrite(shift_pin, LOW);
  digitalWrite(store_pin, LOW);
  digitalWrite(trans_pin1, LOW);
  digitalWrite(trans_pin2, LOW);

}

void shift(int data_pin, int shift_pin, int value) {

  digitalWrite(data_pin, value);
  digitalWrite(shift_pin, LOW);
  digitalWrite(shift_pin, HIGH);

}

void push(int data_pin, int shift_pin, int arr[]) {

  for(int i = 7; i > 0; i--) {
    shift(data_pin, shift_pin, arr[i]);
  }
  digitalWrite(store_pin, LOW);
  digitalWrite(store_pin, HIGH);

}

void loop() {

  push(data_pin, shift_pin, test[led]);

  led++;
  if(led > 8) led = 0;

  delay(2000);

}
