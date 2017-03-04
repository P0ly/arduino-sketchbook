int data_pins[] = {2,3,4,5,6,7,8,9};
int trans_pin1 = 10;
int trans_pin2 = 11;
int temp_pin = A0;
int led = 0;

int test[][8] = {
  {1,0,0,0,0,0,0,0},
  {1,1,0,0,0,0,0,0},
  {1,1,1,0,0,0,0,0},
  {1,1,1,1,0,0,0,0},
  {1,1,1,1,1,0,0,0},
  {1,1,1,1,1,1,0,0},
  {1,1,1,1,1,1,1,0},
  {1,1,1,1,1,1,1,1}
};

void setup() {

  for(int i = 0; i <= 7; i++) {
    pinMode(data_pins[i], OUTPUT);
  }
  pinMode(trans_pin1, OUTPUT);
  pinMode(trans_pin2, OUTPUT);

  digitalWrite(trans_pin1, LOW);
  digitalWrite(trans_pin2, LOW);

}

void push(int data_pins[], int arr[]) {

  for(int i = 0; i <= 7; i++) {
    digitalWrite(data_pins[i], arr[i]);
  }

}

void loop() {

  push(data_pins, test[led]);
  digitalWrite(trans_pin1, HIGH);
  digitalWrite(trans_pin2, LOW);
  delay(1000);

  push(data_pins, test[led]);
  digitalWrite(trans_pin1, LOW);
  digitalWrite(trans_pin2, HIGH);
  delay(1000);

  led++;
  if(led > 7) led = 0;

}
