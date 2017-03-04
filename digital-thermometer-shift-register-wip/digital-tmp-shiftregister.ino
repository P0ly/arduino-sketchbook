const int data_pin = 2;
const int shift_pin = 3;
const int store_pin = 4;
const int trans_pin1 = 5;
const int trans_pin2 = 6;
const int temp_pin = A0;
float vol = 0;
int temp = 0;

int display_number[][8] = {
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

  digitalWrite(trans_pin1, LOW);
  digitalWrite(trans_pin2, HIGH);

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

  vol = (float)(analogRead(temp_pin)) / 1024.0 * 5.0;
  temp = (int)((vol - 0.5) * 100);

  push(data_pin, shift_pin, display_number[temp / 10]);
  digitalWrite(trans_pin1, HIGH);
  digitalWrite(trans_pin2, LOW);

  push(data_pin, shift_pin, display_number[temp % 10]);
  digitalWrite(trans_pin1, LOW);
  digitalWrite(trans_pin2, HIGH);

  delay(1000);

}
