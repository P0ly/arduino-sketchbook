#include <time.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DHT.h>
#include <PubSubClient.h>
#include "secrets.h"
#include "config.h"

#define READ_SENSORS_INTERVAL_MS 300000
#define MQTT_SERVER "homer"
#define TOPIC_BASE "homeassistant/sensor/" ENTITY_ID
#define TOPIC_STATE TOPIC_BASE "/state"
#define TOPIC_AVAILABLE TOPIC_BASE "/available"
#define MSG_FORMAT "{\"temp\": %2.1f,\"humi\": %2.1f, \"hi\": %2.1f}"
#define MSG_CONFIG_FORMAT "{\"dev_cla\": \"%s\",\"name\": \"" FRIENDLY_NAME " %s\",\"stat_t\": \"" TOPIC_STATE "\",\"unit_of_meas\": \"%s\",\"val_tpl\": \"{{ value_json.%s }}\",\"avty_t\": \"" TOPIC_AVAILABLE "\",\"pl_avail\": \"online\",\"pl_not_avail\": \"offline\",\"uniq_id\": \"" ENTITY_ID "_%s\",\"dev\": {\"ids\": [\"%s\",\"%s\"],\"name\": \"" ENTITY_ID "\",\"mdl\": \"" SENSOR_NAME "\"},\"icon\": \"%s\"}"
// "temperature", "Temperatur", "°C", "temp", temp", SERIAL_NO, "mdi:thermometer"
char* DEVICE_ID;

// sensor
#define SENSOR_NAME "AM2302"
#define TOPIC_CONF_TEMP TOPIC_BASE "_temp/config"
#define TOPIC_CONF_HUMI TOPIC_BASE "_humi/config"
#define TOPIC_CONF_HI TOPIC_BASE "_hi/config"
#define DHT_TYPE DHT22 // DHT 22  (AM2302), AM2321
#define DHT_PIN D5
DHT dht(DHT_PIN, DHT_TYPE);

// #define DHT_2
#if defined(DHT_2)
#define TOPIC_CONF_TEMP_2 TOPIC_BASE "_temp_2/config"
#define TOPIC_CONF_HUMI_2 TOPIC_BASE "_humi_2/config"
#define TOPIC_CONF_HI_2 TOPIC_BASE "_hi_2/config"
#define MSG_FORMAT "{\"temp\": %2.1f,\"humi\": %2.1f, \"hi\": %2.1f, \"temp_2\": %2.1f,\"humi_2\": %2.1f, \"hi_2\": %2.1f}"
#define DHT_2_PIN D6
#define DHT_2_TYPE DHT22
DHT dht2(DHT_2_PIN, DHT_2_TYPE);
#endif

// mqtt
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (1000)
char msg[MSG_BUFFER_SIZE];

long timestamp = 0L;
float dht_temp = 0;
float dht_humi = 0;
float dht_hi = 0;
#if defined(DHT_2)
float dht_2_temp = 0;
float dht_2_humi = 0;
float dht_2_hi = 0;
#endif
void read_sensor_samples()
{
  timestamp = time(nullptr);

  dht_temp = dht.readTemperature();
  dht_humi = dht.readHumidity();
  dht_hi = dht.computeHeatIndex(dht_temp, dht_humi, false);

#if defined(DHT_2)
  dht_2_temp = dht2.readTemperature();
  dht_2_humi = dht2.readHumidity();
  dht_2_hi = dht2.computeHeatIndex(dht_2_temp, dht_2_humi, false);
#endif
}

void publish_samples()
{

    read_sensor_samples();
    Serial.printf("%ld: {\"temp\": %2.1f,\"humi\": %2.1f, \"hi\": %2.1f}\n", timestamp, dht_temp, dht_humi, dht_hi);
#if defined(DHT_2)
    snprintf(msg, MSG_BUFFER_SIZE, MSG_FORMAT, dht_temp, dht_humi, dht_hi, dht_2_temp, dht_2_humi, dht_2_hi);
#else
    snprintf(msg, MSG_BUFFER_SIZE, MSG_FORMAT, dht_temp, dht_humi, dht_hi);
#endif
    if (!client.publish(TOPIC_STATE, msg))
      Serial.printf("publish to %s failed\n", TOPIC_STATE);
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
    // config dht_temp
    snprintf(msg, MSG_BUFFER_SIZE, MSG_CONFIG_FORMAT, "temperature", "Temperatur", "°C", "temp", "temp", SERIAL_NO, DEVICE_ID, "mdi:thermometer");
    if (!client.publish(TOPIC_CONF_TEMP, msg, true))
      Serial.printf("publish to %s failed\n", TOPIC_CONF_TEMP);
    // config dht_humi
    snprintf(msg, MSG_BUFFER_SIZE, MSG_CONFIG_FORMAT, "humidity", "Luftfeuchtigkeit", "%", "humi", "humi", SERIAL_NO, DEVICE_ID, "mdi:water-percent");
    if (!client.publish(TOPIC_CONF_HUMI, msg, true))
      Serial.printf("publish to %s failed\n", TOPIC_CONF_HUMI);
    // config dht_hi
    snprintf(msg, MSG_BUFFER_SIZE, MSG_CONFIG_FORMAT, "temperature", "Hitzeindex", "°C", "hi", "hi", SERIAL_NO, DEVICE_ID, "mdi:thermometer");
    if (!client.publish(TOPIC_CONF_HI, msg, true))
      Serial.printf("publish to %s failed\n", TOPIC_CONF_HI);
#if defined(DHT_2)
    // config dht_2_temp
    snprintf(msg, MSG_BUFFER_SIZE, MSG_CONFIG_FORMAT, "temperature", "Außentemperatur", "°C", "temp_2", "temp_outdoor", SERIAL_NO_2, DEVICE_ID, "mdi:thermometer");
    
    if (!client.publish(TOPIC_CONF_TEMP_2, msg, true))
      Serial.printf("publish to %s failed\n", TOPIC_CONF_TEMP_2);
    // config dht_2_humi
    snprintf(msg, MSG_BUFFER_SIZE, MSG_CONFIG_FORMAT, "humidity", "Außenluftfeuchtigkeit", "%", "humi_2", "humi_outdoor", SERIAL_NO_2, DEVICE_ID, "mdi:water-percent");
    if (!client.publish(TOPIC_CONF_HUMI_2, msg, true))
      Serial.printf("publish to %s failed\n", TOPIC_CONF_HUMI_2);
    // config dht_2_hi
    snprintf(msg, MSG_BUFFER_SIZE, MSG_CONFIG_FORMAT, "temperature", "Außenhitzeindex", "°C", "hi_2", "hi_outdoor", SERIAL_NO_2, DEVICE_ID, "mdi:thermometer");
    if (!client.publish(TOPIC_CONF_HI_2, msg, true))
      Serial.printf("publish to %s failed\n", TOPIC_CONF_HI_2);
#endif
    // set status to online
    if (!client.publish(TOPIC_AVAILABLE, "online", true))
      Serial.printf("publish to %s failed\n", TOPIC_AVAILABLE);

    // publish sensor data
    publish_samples();
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
  DEVICE_ID = (char*) malloc(sizeof(char)*(ArduinoOTA.getHostname().length()+1));
  strcpy(DEVICE_ID, ArduinoOTA.getHostname().c_str());
  Serial.println(DEVICE_ID);

  // setup dht22
  pinMode(DHT_PIN, INPUT_PULLUP);
  dht.begin();
#if defined(DHT_2)
  pinMode(DHT_2_PIN, INPUT_PULLUP);
  dht2.begin();
#endif

  // setup mqtt
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  if (!client.setBufferSize(MSG_BUFFER_SIZE))
    Serial.println("the buffer could not be resized");
  publish_samples();
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
    publish_samples();
  }
}