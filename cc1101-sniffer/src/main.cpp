#include <Arduino.h>
#include <RadioLib.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <time.h>

#include "secrets.h"

// CC1101 has the following connections:
#define CS_PIN GPIO_NUM_16 // G16
#define GDO0_PIN GPIO_NUM_17 // G17
#define RST_PIN RADIOLIB_NC // unused
#define GDO2_PIN GPIO_NUM_5 // G5 (optional)
CC1101 cc = new Module(CS_PIN, GDO0_PIN, RADIOLIB_NC, GDO2_PIN);

#define PACKET_BUFFER_SIZE 100
String packets[PACKET_BUFFER_SIZE];
unsigned long packet_timestamps[PACKET_BUFFER_SIZE] = {0};
float packet_rssis[PACKET_BUFFER_SIZE] = {0};
uint8_t packet_lqis[PACKET_BUFFER_SIZE] = {0};
int packets_p = 0;

AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void) {
  // check if the interrupt is enabled
  if(!enableInterrupt) {
    return;
  }

  // we got a packet, set the flag
  receivedFlag = true;
}

void setup() {

  Serial.begin(115200);

  // setup wifi
  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PW);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
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
    delay(500);
  }
  time_t now = time(nullptr);
  Serial.println(now);
  Serial.println(ctime(&now));

  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  Serial.println(WiFi.softAP("ESP32-1","baCon123") ? "Ready" : "Failed!");

  // initialize CC1101 with default settings
  // carrier frequency:                   868.0 MHz
  // bit rate:                            4.8 kbps
  // frequency deviation:                 48.0 kHz
  // Rx bandwidth:                        325.0 kHz
  // sync word:                           0xD391

  // initialize CC1101 with non-default settings
  Serial.print(F("[CC1101] Initializing ... "));
  // carrier frequency:                   434.0 MHz
  // bit rate:                            32.0 kbps
  // frequency deviation:                 60.0 kHz
  // Rx bandwidth:                        250.0 kHz
  // sync word:                           0xD391
  int16_t state = cc.begin(869.525f, 4.8f, 48.0f, 325.0f);
  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // set the function that will be called
  // when new packet is received
  cc.setGdo0Action(setFlag);

  // start listening for packets
  Serial.print(F("[CC1101] Starting to listen ... "));
  state = cc.startReceive();
  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // you can also change the settings at runtime
  // and check if the configuration was changed successfully

  // set carrier frequency to 433.5 MHz
  // if (cc.setFrequency(433.5) == ERR_INVALID_FREQUENCY) {
  //   Serial.println(F("[CC1101] Selected frequency is invalid for this module!"));
  //   while (true);
  // }

  // set bit rate to 100.0 kbps
  // state = cc.setBitRate(100.0);
  // if (state == ERR_INVALID_BIT_RATE) {
  //   Serial.println(F("[CC1101] Selected bit rate is invalid for this module!"));
  //   while (true);
  // } else if (state == ERR_INVALID_BIT_RATE_BW_RATIO) {
  //   Serial.println(F("[CC1101] Selected bit rate to bandwidth ratio is invalid!"));
  //   Serial.println(F("[CC1101] Increase receiver bandwidth to set this bit rate."));
  //   while (true);
  // }

  // // set receiver bandwidth to 250.0 kHz
  // if (cc.setRxBandwidth(250.0) == ERR_INVALID_RX_BANDWIDTH) {
  //   Serial.println(F("[CC1101] Selected receiver bandwidth is invalid for this module!"));
  //   while (true);
  // }

  // // set allowed frequency deviation to 10.0 kHz
  // if (cc.setFrequencyDeviation(10.0) == ERR_INVALID_FREQUENCY_DEVIATION) {
  //   Serial.println(F("[CC1101] Selected frequency deviation is invalid for this module!"));
  //   while (true);
  // }

  // // set output power to 5 dBm
  // if (cc.setOutputPower(5) == ERR_INVALID_OUTPUT_POWER) {
  //   Serial.println(F("[CC1101] Selected output power is invalid for this module!"));
  //   while (true);
  // }

  // // 2 bytes can be set as sync word
  // if (cc.setSyncWord(0x01, 0x23) == ERR_INVALID_SYNC_WORD) {
  //   Serial.println(F("[CC1101] Selected sync word is invalid for this module!"));
  //   while (true);
  // }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String body;
    for(int i = 0; i < PACKET_BUFFER_SIZE && packet_timestamps[i] != 0; i++) {
      body += "<p>Packet ";
      body += i;
      body += "<br>";
      body += packet_timestamps[i];
      body += "<br>Length: ";
      body += packets[i].length();
      body += "<br>RSSI: ";
      body += packet_rssis[i];
      body += "<br>LQI: ";
      body += packet_lqis[i];
      body += "<br><a href=\"/packet?id=";
      body += i;
      body += "\">Data</a>";
      body += "</p><hr>";
    }
    request->send(200,"text/html",body);
  });

  server.on("/packet", HTTP_GET, [](AsyncWebServerRequest *request){
    int id = (int)request->getParam("id")->value().toInt();
    // Serial.print("GET /packet?id=");
    // Serial.println(id);
    if(id >= PACKET_BUFFER_SIZE) {
      request->send(404);
      return;
    }
    // Serial.println(String(packets[id]));
    // Serial.println();
    request->send(200, "application/octet-stream",  packets[id]);
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Not found");
  });

  server.begin();

}

void loop() {

  // check if the flag is set
  if(receivedFlag) {
    // disable the interrupt service routine while
    // processing the data
    enableInterrupt = false;

    // reset flag
    receivedFlag = false;

    // you can read received data as an Arduino String
    String data;
    int state = cc.readData(data);

    // you can also read received data as byte array
    // byte byteArr[8];
    // int state = cc.readData(byteArr, 8);

    if (state == ERR_NONE) {
      // packet was successfully received
      Serial.print(F("Received packet "));
      Serial.println(packets_p);
      packet_timestamps[packets_p] = time(nullptr);

      // print data of the packet
      //Serial.print(F("[CC1101] Data:\t\t"));
      //Serial.println(str);
      packets[packets_p].~String();
      packets[packets_p] = data;
      Serial.print("Length: ");
      Serial.println(data.length());

      // print RSSI (Received Signal Strength Indicator)
      // of the last received packet
      Serial.print(F("RSSI: "));
      float rssi = cc.getRSSI();
      Serial.print(rssi);
      Serial.println(F(" dBm"));
      packet_rssis[packets_p] = rssi;

      // print LQI (Link Quality Indicator)
      // of the last received packet, lower is better
      uint8_t lqi = cc.getLQI();
      Serial.print(F("LQI: ")); 
      Serial.println(lqi);
      packet_lqis[packets_p] = lqi;
      Serial.println();

      packets_p++;
      if(packets_p > PACKET_BUFFER_SIZE)
        packets_p = 0;

    } else if (state == ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
      Serial.println(F("CRC error!"));
      Serial.println();

    } else {
      // some other error occurred
      Serial.print(F("failed, code "));
      Serial.println(state);

    }

    // put module back to listen mode
    cc.startReceive();

    // we're ready to receive more packets,
    // enable interrupt service routine
    enableInterrupt = true;
  }
}