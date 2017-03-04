const int data_pin = 2;
const int shift_pin = 3;
const int store_pin = 4;
const int trans_pin1 = 5;
const int trans_pin2 = 6;
const int temp_pin = A0;
int num = 0;

const int display_number[][8] = {
  {1,1,1,1,0,0,1,1}, // 0
  {1,0,0,1,0,0,0,0}, // 1
  {1,1,1,0,0,1,0,1}, // 2
  {1,0,1,1,0,1,0,1}, // 3
  {1,1,0,0,0,1,1,0}, // 4
  {0,1,1,0,0,1,1,1}, // 5
  {0,1,1,1,0,1,1,1}, // 6
  {1,0,0,1,0,0,0,1}, // 7
  {1,1,1,1,0,1,1,1}, // 8
  {1,0,1,1,0,1,1,1}  // 9
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

  push(data_pin, shift_pin, display_number[num]);

  num++;
  if(num > 8) num = 0;

  delay(2000);

}
