int data_pins[] = {2,3,4,5,6,7,8,9};
int trans_pin1 = 10;
int trans_pin2 = 11;
int temp_pin = A0;
int sps = 1000; // scans per second
int delays = 10;

float vol = 0;
int temp = 0;
int temp1 = 0;
int temp2 = 0;
unsigned long start_time = 0;

int display_number[][8] = {
  {0,1,1,1,1,1,1,0}, // 0
  {0,0,0,1,0,0,1,0}, // 1
  {1,0,1,1,1,1,0,0}, // 2
  {1,0,1,1,0,1,1,0}, // 3
  {1,1,0,1,0,0,1,0}, // 4
  {1,1,1,0,0,1,1,0}, // 5
  {1,1,1,0,1,1,1,0}, // 6
  {0,0,1,1,0,0,1,0}, // 7
  {1,1,1,1,1,1,1,0}, // 8
  {1,1,1,1,0,1,1,0}  // 9
};

void setup() {

  for(int i = 0; i <= 7; i++) {
    pinMode(data_pins[i], OUTPUT);
  }
  pinMode(trans_pin1, OUTPUT);
  pinMode(trans_pin2, OUTPUT);

  digitalWrite(trans_pin1, LOW);
  digitalWrite(trans_pin2, LOW);

  start_time = millis();

}

void push(int data_pins[], int arr[]) {

  for(int i = 0; i <= 7; i++) {
    digitalWrite(data_pins[i], arr[i]);
  }

}

void loop() {

  if(millis() - start_time > sps) {

    vol = (float)(analogRead(temp_pin)) / 1024.0 * 5.0;
    temp = (int)((vol - 0.5) * 100);
    temp1 = temp / 10;
    temp2 = temp % 10;

    start_time = millis();

  }

  push(data_pins, display_number[temp1]);
  digitalWrite(trans_pin1, HIGH);
  digitalWrite(trans_pin2, LOW);
  delay(delays);

  push(data_pins, display_number[temp2]);
  digitalWrite(trans_pin1, LOW);
  digitalWrite(trans_pin2, HIGH);
  delay(delays);

}
