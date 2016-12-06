#include <LiquidCrystal.h>

/* CCONFIG */
const int tmpPin = A0;
const int tick = 1000;

/* GLOBAL VAR */
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
float tmpV, tmpC;
int counter = 0;

void setup() {

  lcd.begin(16,2);

}

float getTemp() {

  unsigned int wADC;
  float t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celsius.
  return (t);

}

void loop() {

  tmpV = (analogRead(tmpPin)/1024.0) * 5.0;
  tmpC = (tmpV - 0.5) * 100;

  // lcd.clear();
  lcd.setCursor(0,0);
  // lcd.print("V:");
  // lcd.print(tmpV);
  lcd.print("C:");
  lcd.print(tmpC);
  lcd.print(" iC:");
  lcd.print(getTemp());

  counter++;
  lcd.setCursor(0,1);
  for(int i = 0; i < 16; i++) {
    lcd.print(counter >> 16-i-1 & 1);
  }

  delay(tick);

}
