#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <DHT.h>
#include <time.h>

#include "secrets.h"

#define READ_SENSORS_INTERVAL_MS 30000
#define SENSOR_SAMPLES_BUFFER_SIZE 20

#define BME_ADR 0x76
#define BME_REAL_ALTITUDE 997
Adafruit_BME280 bme;

#define DHTPIN D3
#define DHTTYPE DHT22 // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

ESP8266WebServer server(80);

// float bme_temp = 0;
// float bme_humi = 0;
// float bme_alti = 0;
// float bme_pres = 0;
// float dht_temp = 0;
// float dht_humi = 0;
// float dht_hic = 0;

// 22 byte per sample
float bme_samples[SENSOR_SAMPLES_BUFFER_SIZE][4] = {0}; // [Pressure, Altitude, Temperature, Humidity]
long bme_samples_timestamps[SENSOR_SAMPLES_BUFFER_SIZE] = {0};
int bme_samples_counter = 0;
float dht_samples[SENSOR_SAMPLES_BUFFER_SIZE][3] = {0}; // [Temperature, Humidity, Heat Index]
long dht_samples_timestamps[SENSOR_SAMPLES_BUFFER_SIZE] = {0};
int dht_samples_counter = 0;


// void read_sensor_values()
// {

//   dht_humi = dht.readHumidity();
//   dht_temp = dht.readTemperature();
//   dht_hic = dht.computeHeatIndex(dht_temp, dht_humi, false);

//   bme_temp = bme.readTemperature();
//   bme_humi = bme.readHumidity();
//   bme_pres = bme.readPressure() / 100.0f;
//   bme_alti = bme.readAltitude(BME_REAL_ALTITUDE);

// }

void read_sensor_samples()
{

  bme_samples_timestamps[bme_samples_counter] = time(nullptr);
  bme_samples[bme_samples_counter][0] = bme.readPressure() / 100.0f;
  bme_samples[bme_samples_counter][1] = bme.readAltitude(BME_REAL_ALTITUDE);
  bme_samples[bme_samples_counter][2] = bme.readTemperature();
  bme_samples[bme_samples_counter][3] = bme.readHumidity();
  bme_samples_counter++;
  if (bme_samples_counter > SENSOR_SAMPLES_BUFFER_SIZE)
    bme_samples_counter = 0;

  dht_samples_timestamps[dht_samples_counter] = time(nullptr);
  float dht_temp = bme.readTemperature();
  float dht_humi = bme.readHumidity();
  dht_samples[dht_samples_counter][0] = dht_temp;
  dht_samples[dht_samples_counter][1] = dht_humi;
  dht_samples[dht_samples_counter][2] = dht.computeHeatIndex(dht_temp, dht_humi, false);
  dht_samples_counter++;
  if (dht_samples_counter > SENSOR_SAMPLES_BUFFER_SIZE)
    dht_samples_counter = 0;

}

void setup()
{

  system_update_cpu_freq(80);

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

  // setup bme280
  // You can also pass in a Wire library object like &Wire2
  if (!bme.begin(BME_ADR))
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x");
    Serial.println(bme.sensorID(), 16);
  }

  // setup dht22
  pinMode(DHTPIN, INPUT_PULLUP);
  dht.begin();

  read_sensor_samples();

  // web server
  server.on("/", []() {
    
    unsigned long start_time = millis();

    // [Pressure, Altitude, Temperature, Humidity]
    String body = "<a href=\"/bme280.csv\"><h2>BME280</h2></a><p>";
    body += bme_samples[bme_samples_counter-1][0];
    body += " hPa <br>";
    body += bme_samples[bme_samples_counter-1][1];
    body += " m <br>";
    body += bme_samples[bme_samples_counter-1][2];
    body += " °C <br>";
    body += bme_samples[bme_samples_counter-1][3];
    body += " % </p><hr>";

    // [Temperature, Humidity, Heat Index]
    body += "<a href=\"/dht22.csv\"><h2>DHT22</h2></a><p>";
    body += dht_samples[dht_samples_counter-1][0];
    body += " °C <br>";
    body += dht_samples[dht_samples_counter-1][1];
    body += " % <br>Heat Index: ";
    body += dht_samples[dht_samples_counter-1][2];
    body += " °C</p>";

    server.send(200, "text/html; charset=UTF-8", body);

    Serial.print("serving / took ");
    Serial.println(millis() - start_time);
  
  });

  server.on("/bme280.csv", []() {

    unsigned long start_time = millis();

    String csv = "Timestamp,Pressure,Altitude,Temperature,Humidity\n";
    for (int i = 0; i < SENSOR_SAMPLES_BUFFER_SIZE; i++)
    {

      if(bme_samples_timestamps[i] == 0)
        break;
      csv += bme_samples_timestamps[i];
      csv += ",";
      csv += bme_samples[i][0];
      csv += ",";
      csv += bme_samples[i][1];
      csv += ",";
      csv += bme_samples[i][2];
      csv += ",";
      csv += bme_samples[i][3];
      csv += "\n";

    }

    server.send(200, "text/csv; charset=UTF-8", csv);

    Serial.print("serving /bme280.csv took ");
    Serial.println(millis() - start_time);

  });

  server.on("/dht22.csv", []() {

    unsigned long start_time = millis();

    String csv = "Timestamp,Temperature,Humidity,Heat-Index\n";
    for (int i = 0; i < SENSOR_SAMPLES_BUFFER_SIZE; i++)
    {

      if(dht_samples_timestamps[i] == 0)
        break;
      csv += dht_samples_timestamps[i];
      csv += ",";
      csv += dht_samples[i][0];
      csv += ",";
      csv += dht_samples[i][1];
      csv += ",";
      csv += dht_samples[i][2];
      csv += "\n";

    }

    server.send(200, "text/csv; charset=UTF-8", csv);

    Serial.print("serving /dht22.csv took ");
    Serial.println(millis() - start_time);

  });

  server.onNotFound([]() {
    server.send(404, "text/plain; charset=UTF-8", "Dies ist nicht die Seite die du suchst.\nDu wirst jetzt nach Hause gehen und dein Leben überdenken.");
  });

  server.begin();
}

unsigned long last_update = 0;
unsigned long start_time = 0;
void loop()
{

  server.handleClient();

  if (millis() - last_update >= READ_SENSORS_INTERVAL_MS)
  {

    last_update = millis();
    
    start_time = millis();
    read_sensor_samples();
    Serial.print("sensor values read in ");
    Serial.println(millis() - start_time);

  }

}