const int DATA = 2, LATCH = 3, CLOCK = 4;
const int COL_PINS[] = {5,6,7};

const int UPS = 1/60*1000;

byte col_data[] = {1,2,3};
int column = 0;

void update_register(int id) {
  
  digitalWrite(LATCH, LOW);
  shiftOut(DATA, CLOCK, LSBFIRST, ~col_data[id]);
  digitalWrite(LATCH, HIGH);
  
}

void setup() {

  pinMode(DATA ,OUTPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  for(int i = 0; i < sizeof(COL_PINS); i++) {
    pinMode(COL_PINS[i], OUTPUT);
    digitalWrite(COL_PINS[i], HIGH);
  }

  for(int i = 0; i < sizeof(COL_PINS); i++) {
    update_register(i);
  }

}

void loop() {

  //col_data[0] = 0b01111111, col_data[1] = 0b01111111, col_data[2] = 0b01111111;

  update_register(column);
  
  for(int i = 0; i < sizeof(COL_PINS); i++) {
    digitalWrite(COL_PINS[i], HIGH);
  }
    
  digitalWrite(COL_PINS[column], LOW);
  
  column++;
  if(column > sizeof(COL_PINS)) {
    column = 0;
  }
  
  delay(UPS);

}
