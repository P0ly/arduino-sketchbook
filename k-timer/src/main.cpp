#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <Ticker.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "secrets.h"

#define PLUS_PIN D5
#define MINUS_PIN D6
#define OK_PIN D7
#define BUZZER_PIN D8

#define HOLD_DURATION_MS 500
#define HOLD_COOLDOWN_MS 50
#define HOLD_OK_DURATION_MS 2000
#define HOLD_OK_COOLDOWN_MS 1000
#define HISTORY_SIZE 50
#define MAX_COUNTER_TIME_S 6000-59

#define FONT_HEIGHT 8
#define FONT_WIDTH 6
#define SCREEN_ADR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

ADC_MODE(ADC_VCC);

Ticker ticker_counter;
uint16_t counter = 0;
uint16_t history[HISTORY_SIZE] = {0};
uint16_t history_p = 0;
uint16_t history_fav[3] = {0};
uint16_t history_fav_p = 0;

bool display_flag = true;
bool history_select_flag = false;

enum State
{
  SET,
  COUNTDOWN,
  RING
};
State state = SET;

bool is_minus_pressed = false;
bool is_plus_pressed = false;
bool is_ok_pressed = false;
unsigned long minus_pressed_time = 0;
unsigned long plus_pressed_time = 0;
unsigned long ok_pressed_time = 0;

// const int RING_TONES[] = {1500,1500,1500,0};
// const unsigned long RING_DURATIONS[] = {500,500,500,0};
// const unsigned long RING_TIMINGS[] = {1000,1000,2000};

// mario theme
const unsigned long RING_TIMINGS[] = {150, 300, 300, 100, 300, 550, 575, 450, 400, 500, 300, 330, 150, 300, 200, 200, 150, 300, 150, 350, 300, 150, 150, 500, 450, 400, 500, 300, 330, 150, 300, 200, 200, 150, 300, 150, 350, 300, 150, 150, 500, 300, 100, 150, 150, 300, 300, 150, 150, 300, 150, 100, 220, 300, 100, 150, 150, 300, 300, 300, 150, 300, 300, 300, 100, 150, 150, 300, 300, 150, 150, 300, 150, 100, 420, 450, 420, 360, 300, 300, 150, 300, 300, 100, 150, 150, 300, 300, 150, 150, 300, 150, 100, 220, 300, 100, 150, 150, 300, 300, 300, 150, 300, 300, 300, 100, 150, 150, 300, 300, 150, 150, 300, 150, 100, 420, 450, 420, 360, 300, 300, 150, 300, 150, 300, 350, 150, 350, 150, 300, 150, 600, 150, 300, 350, 150, 150, 550, 325, 600, 150, 300, 350, 150, 350, 150, 300, 150, 600, 150, 300, 300, 100, 300, 550, 575};
const int RING_TONES[] = {660, 660, 660, 510, 660, 770, 380, 510, 380, 320, 440, 480, 450, 430, 380, 660, 760, 860, 700, 760, 660, 520, 580, 480, 510, 380, 320, 440, 480, 450, 430, 380, 660, 760, 860, 700, 760, 660, 520, 580, 480, 500, 760, 720, 680, 620, 650, 380, 430, 500, 430, 500, 570, 500, 760, 720, 680, 620, 650, 1020, 1020, 1020, 380, 500, 760, 720, 680, 620, 650, 380, 430, 500, 430, 500, 570, 585, 550, 500, 380, 500, 500, 500, 500, 760, 720, 680, 620, 650, 380, 430, 500, 430, 500, 570, 500, 760, 720, 680, 620, 650, 1020, 1020, 1020, 380, 500, 760, 720, 680, 620, 650, 380, 430, 500, 430, 500, 570, 585, 550, 500, 380, 500, 500, 500, 500, 500, 500, 500, 580, 660, 500, 430, 380, 500, 500, 500, 500, 580, 660, 870, 760, 500, 500, 500, 500, 580, 660, 500, 430, 380, 660, 660, 660, 510, 660, 770, 380};
const unsigned long RING_DURATIONS[] = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 80, 100, 100, 100, 80, 50, 100, 80, 50, 80, 80, 80, 80, 100, 100, 100, 100, 80, 100, 100, 100, 80, 50, 100, 80, 50, 80, 80, 80, 80, 100, 100, 100, 100, 150, 150, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 150, 200, 80, 80, 80, 100, 100, 100, 100, 100, 150, 150, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 150, 150, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 150, 200, 80, 80, 80, 100, 100, 100, 100, 100, 150, 150, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 60, 80, 60, 80, 80, 80, 80, 80, 80, 60, 80, 60, 80, 80, 80, 80, 80, 60, 80, 60, 80, 80, 80, 80, 80, 80, 100, 100, 100, 100, 100, 100, 100};

// button control functions
ICACHE_RAM_ATTR void increase_counter()
{
  if (counter < MAX_COUNTER_TIME_S - 1)
  {
    counter += 60;
  }
  else
  {
    counter = 0;
  }
  display_flag = true;
}

ICACHE_RAM_ATTR void plus_pressed()
{

  if (digitalRead(PLUS_PIN) == LOW)
  {
    is_plus_pressed = true;
    plus_pressed_time = millis();

    if (state == SET)
    {
      increase_counter();
    }
    else if (state == RING)
    {
      noTone(BUZZER_PIN);
      state = SET;
    }
  }
  else
  {
    is_plus_pressed = false;
  }

  Serial.print("plus: ");
  Serial.println(is_plus_pressed);
}

ICACHE_RAM_ATTR void decrease_counter()
{
  if (counter > 0)
  {
    counter -= 60;
  }
  else
  {
    counter = MAX_COUNTER_TIME_S - 1;
  }
  display_flag = true;
}

ICACHE_RAM_ATTR void minus_pressed()
{

  if (digitalRead(MINUS_PIN) == LOW)
  {
    is_minus_pressed = true;
    minus_pressed_time = millis();

    if (state == SET)
    {
      decrease_counter();
    }
    else if (state == RING)
    {
      noTone(BUZZER_PIN);
      state = SET;
    }
  }
  else
  {
    is_minus_pressed = false;
  }

  Serial.print("minus: ");
  Serial.println(is_minus_pressed);
}

ICACHE_RAM_ATTR void push_to_history(uint16_t counter)
{

  if(history_p >= HISTORY_SIZE)
    history_p = 0;
  history[history_p++] = counter;

  // TODO sort favorites by count
  // uint16_t counts[HISTORY_SIZE] = {0};
  // uint16_t counts_id[HISTORY_SIZE] = {0};
  // uint16_t counts_id_p = 0;
  // for(int i = 0; i < HISTORY_SIZE; i++)
  // {
  //   if(history[i] = 0)
  //     break;
  //   for(int j = 0; j < HISTORY_SIZE; j++) {
  //     if(history[i] = 0)
  //       break;
  //     if(history[i] = counts_id[i]) {
  //       counts[i]++;
  //     }
  //     else
  //     {
  //       counts_id[counts_id_p++] = history[i];
  //     }
  //   }
  // }

  
  history_fav[0] = history[history_p - 1];
  if(history_p == 0)
  {
    history_fav[1] = history[HISTORY_SIZE - 2];
    history_fav[2] = history[HISTORY_SIZE - 3];
  }
  else if(history_p == 1)
  {
    history_fav[1] = history[HISTORY_SIZE - 3];
    history_fav[2] = history[HISTORY_SIZE - 4];
  }
  else
  {
    history_fav[1] = history[history_p - 2];
    history_fav[2] = history[history_p - 3];
  }

}

ICACHE_RAM_ATTR void countdown()
{
  counter--;
  if (counter <= 0)
  {
    ticker_counter.detach();
    state = RING;
  }
  display_flag = true;
}

ICACHE_RAM_ATTR void ok_pressed()
{
  if (digitalRead(OK_PIN) == LOW)
  {
    is_ok_pressed = true;
    ok_pressed_time = millis();

    if (state == SET)
    {
      if (counter != 0 && !history_select_flag)
      {
        push_to_history(counter); // add to history
        ticker_counter.attach(1.0f, countdown); // start counter loop
        state = COUNTDOWN;
      }
    }
    else if (state == COUNTDOWN)
    {
      ticker_counter.detach(); // stop counter loop
      state = SET;
    }
    else if (state == RING)
    {
      noTone(BUZZER_PIN); // stop buzzer
      state = SET;
    }
  }
  else
  {
    is_ok_pressed = false;
    history_select_flag = false;
  }

  Serial.print("ok: ");
  Serial.println(is_ok_pressed);

}

ICACHE_RAM_ATTR void reset_counter()
{
  counter = 0;
  display_flag = true;
}

void setup()
{

  // system setup
  system_update_cpu_freq(80);
  pinMode(PLUS_PIN, INPUT_PULLUP);
  pinMode(MINUS_PIN, INPUT_PULLUP);
  pinMode(OK_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(115200);

  // setup wifi
  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PW);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  // get time from ntp server
  configTime(1 * 3600, 0, "pool.ntp.org");
  Serial.println("\nWaiting for ntp server");
  while (!time(nullptr))
  {
    Serial.print(".");
    delay(1000);
  }
  time_t now = time(nullptr);
  Serial.println(now);
  Serial.println(ctime(&now));

  // setup OTA
  ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("hostname");
  // No authentication by default
  // ArduinoOTA.setPassword(SECRET_OTA_PW);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println(ArduinoOTA.getHostname());

  // turn off wifi
  // WiFi.mode(WIFI_OFF);
  // WiFi.forceSleepBegin();
  // delay(1);

  // setup display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADR))
  {
    Serial.println(F("SSD1306 allocation failed"));
  }
  // display text settings
  display.setTextColor(SSD1306_WHITE);
  display.dim(true);

  // button input interrupts
  attachInterrupt(PLUS_PIN, plus_pressed, CHANGE);
  attachInterrupt(MINUS_PIN, minus_pressed, CHANGE);
  attachInterrupt(OK_PIN, ok_pressed, CHANGE);

}

String get_timer_str(uint16_t counter)
{

  // timer
  String timer_str;

  // minutes
  if (counter >= 60)
  {
    if (counter < 10 * 60)
      timer_str += '0';
    timer_str += counter / 60;
    timer_str += ':';
  }
  else
  {
    timer_str += "00:";
  }

  // seconds
  if (counter % 60 < 10)
  {
    timer_str += '0';
  }
  timer_str += counter % 60;

  return timer_str;
}

String get_history_str()
{

  String history_str;
  history_str += get_timer_str(history_fav[0]);
  history_str += '|';
  history_str += get_timer_str(history_fav[1]);
  history_str += '|';
  history_str += get_timer_str(history_fav[2]);
  return history_str;
}

void update_display()
{

  display.clearDisplay();

  // battery
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(ESP.getVcc() / 1000.0);
  display.print('V');

  // timer
  String timer_str = get_timer_str(counter);
  const int timer_test_size = 4;
  display.setTextSize(timer_test_size);
  display.setCursor((SCREEN_WIDTH / 2) - (timer_test_size * FONT_WIDTH * timer_str.length() / 2), (SCREEN_HEIGHT / 2) - (timer_test_size * FONT_HEIGHT / 2));
  display.print(timer_str);

  // history
  String history_str = get_history_str();
  const int history_text_size = 1;
  display.setTextSize(history_text_size);
  display.setCursor((SCREEN_WIDTH / 2) - (history_text_size * FONT_WIDTH * history_str.length() / 2), SCREEN_HEIGHT - (history_text_size * FONT_HEIGHT));
  display.print(history_str);
  // display.drawLine(
  //   (SCREEN_WIDTH / 2) - (history_text_size * FONT_WIDTH * history_str.length() / 2) - (2 * FONT_WIDTH) + (7 * FONT_WIDTH * history_fav_p),
  //   SCREEN_HEIGHT - (history_text_size * FONT_HEIGHT),
  //   (SCREEN_WIDTH / 2) - (history_text_size * FONT_WIDTH * history_str.length() / 2) + (7 * FONT_WIDTH) + (7 * FONT_WIDTH * history_fav_p),
  //   SCREEN_HEIGHT - (history_text_size * FONT_HEIGHT),
  //   SSD1306_WHITE
  // );

  display.display();

}

void loop()
{

  // ota update
  ArduinoOTA.handle();

  // update display if needed
  if (display_flag)
  {

    update_display();
    display_flag = false;

  }

  // check buttons
  if (state == SET)
  {

    if (
        is_plus_pressed && is_minus_pressed &&
        millis() - plus_pressed_time >= HOLD_DURATION_MS && millis() - minus_pressed_time >= HOLD_DURATION_MS)
    {
      reset_counter();
      update_display();
      delay(HOLD_COOLDOWN_MS);
    }
    else if (is_plus_pressed && millis() - plus_pressed_time >= HOLD_DURATION_MS)
    {
      increase_counter();
      update_display();
      delay(HOLD_COOLDOWN_MS);
    }
    else if (is_minus_pressed && millis() - minus_pressed_time >= HOLD_DURATION_MS)
    {
      decrease_counter();
      update_display();
      delay(HOLD_COOLDOWN_MS);
    }
    else if (is_ok_pressed && millis() - ok_pressed_time >= HOLD_OK_DURATION_MS)
    {

      history_select_flag = true;

      // set counter to next history entry
      if(history_fav_p >= 3)
        history_fav_p = 0;
      counter = history[(history_p - 1) - history_fav_p];
      history_fav_p++;
      
      update_display();
      delay(HOLD_OK_COOLDOWN_MS);
    }
  }
  else if (state == RING)
  {
    int ring_size = sizeof(RING_TONES) / sizeof(int);
    for (int i = 0; i < ring_size; i++)
    {
      if (state != RING)
      {
        noTone(BUZZER_PIN);
        break;
      }
      tone(BUZZER_PIN, RING_TONES[i], RING_DURATIONS[i]);
      delay(RING_TIMINGS[i]);
    }
  }
}