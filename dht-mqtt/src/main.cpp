#include <time.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DHT.h>
#include <PubSubClient.h>
#include "secrets.h"

#define READ_SENSORS_INTERVAL_MS 300000
#define MQTT_SERVER "homer"
#define ENTITY_ID "wohnzimmer_sensor"
#define FRIENDLY_NAME "Wohnzimmer"
#define SENSOR_NAME "AM2302"
#define SERIAL_NO ""
#define TOPIC_STATE "homeassistant/sensor/wohnzimmer_sensor/state"
#define TOPIC_AVAILABLE "homeassistant/sensor/wohnzimmer_sensor/available"
#define TOPIC_CONF_TEMP "homeassistant/sensor/wohnzimmer_sensor_temp/config"
#define TOPIC_CONF_HUMI "homeassistant/sensor/wohnzimmer_sensor_humi/config"
#define MSG_FORMAT "{\"temp\": %2.1f,\"humi\": %2.1f}"
#define MSG_CONFIG_FORMAT "{\
                        \"device_class\": '%s',\
                        \"name\": %s + ' %s',\
                        \"state_topic\": %s,\
                        \"unit_of_measurement\": '%s',\
                        \"value_template\": '{{ value_json.%s }}',\
                        \"availability_topic\": %s,\
                        \"payload_available\": 'online',\
                        \"payload_not_available\": 'offline',\
                        \"unique_id\": %s + '_%s_' + %s,\
                        \"device\": {\
                            \"identifiers\": [\
                                %s\
                            ],\
                          \"name\": %s,\
                          \"model\": %s,\
                        },\
                        \"icon\": '%s',\
                      }"

// sensor
#define DHTPIN D5
#define DHTTYPE DHT22 // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

// mqtt
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (1000)
char msg[MSG_BUFFER_SIZE];

long timestamp = 0L;
float dht_temp = 0;
float dht_humi = 0;

void read_sensor_samples()
{
  timestamp = time(nullptr);
  dht_temp = dht.readTemperature();
  dht_humi = dht.readHumidity();
}

void callback(char *topic, byte *payload, unsigned int length)
{
  // handle message arrived
}

boolean reconnect()
{
  Serial.print("Attempting MQTT connection...");
  // Attempt to connect
  if (client.connect(ENTITY_ID, TOPIC_AVAILABLE, 1, true, "offline"))
  {
    Serial.println("connected");
    // Once connected, publish an announcement...

    // auto discovery config
    snprintf(
      msg,
      MSG_BUFFER_SIZE,
      MSG_CONFIG_FORMAT,
      "temperature",
      "Temperatur",
      FRIENDLY_NAME,
      "°C",
      "temp",
      TOPIC_STATE,
      TOPIC_AVAILABLE,
      "temp",
      ENTITY_ID,
      SENSOR_NAME,
      SERIAL_NO,
      ENTITY_ID,
      SENSOR_NAME,
      "mdi:thermometer"
    );
    client.publish(TOPIC_CONF_TEMP, msg, true);

    snprintf(
      msg,
      MSG_BUFFER_SIZE,
      MSG_CONFIG_FORMAT,
      "humidity",
      "Luftfeuchtigkeit",
      FRIENDLY_NAME,
      "%",
      "humi",
      TOPIC_STATE,
      TOPIC_AVAILABLE,
      "humi",
      ENTITY_ID,
      SENSOR_NAME,
      SERIAL_NO,
      ENTITY_ID,
      SENSOR_NAME,
      "mdi:water-percent"
    );
    client.publish(TOPIC_CONF_HUMI, msg, true);

    client.publish(TOPIC_AVAILABLE, "online", true);

    client.publish(TOPIC_STATE, MSG_FORMAT);
  }
  return client.connected();
}

void setup()
{
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
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println(ArduinoOTA.getHostname());

  // setup dht22
  pinMode(DHTPIN, INPUT_PULLUP);
  dht.begin();
  read_sensor_samples();

  // setup mqtt
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
}

unsigned long last_update = 0;
unsigned long start_time = 0;
long lastReconnectAttempt = 0;

void loop()
{
  ArduinoOTA.handle();

  // mqtt reconnect
  if (!client.connected())
  {
    long now = millis();
    if (now - lastReconnectAttempt > 5000)
    {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect())
      {
        lastReconnectAttempt = 0;
      }
    }
  }
  else
  {
    // Client connected
    client.loop();
  }

  if (millis() - last_update >= READ_SENSORS_INTERVAL_MS)
  {
    last_update = millis();
    start_time = millis();
    read_sensor_samples();
    Serial.printf("%ld: {\"temp\": %2.1f,\"humi\": %2.1f}; %dms\n", timestamp, dht_temp, dht_humi, (uint16_t)(millis() - start_time));
    snprintf(msg, MSG_BUFFER_SIZE, MSG_FORMAT, dht_temp, dht_humi);
    client.publish(TOPIC_STATE, msg);
  }
}