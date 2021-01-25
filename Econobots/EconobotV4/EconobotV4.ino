/******************************************************************************
  May 1st, 2020

  Debug 9600

  Protocole: AWS IoT + MQTT + NTP Timestamp
  Sensors: BME280

  6 measures/min


******************************************************************************/

// AWS and WiFi
#include "secrets.h"
//#include "topics.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

// The MQTT topics that this device should publish/subscribe
#define ECONOBOT_CONNECTION "econobotdemo/connection"
#define ECONOBOT_DATA "econobotdemo/data"

WiFiClientSecure net = WiFiClientSecure();
MQTTClient mqttclient = MQTTClient(256);

// Serials
int baud = 9600;
char msg[50];
int value = 0;

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;
int period = 10 * 1000; //interval measure in sec

// BME280
#include <Wire.h> //i2c
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;
#define bme_address 0x76
#define SEALEVELPRESSURE_HPA (1013.25)
float h, t, p, a;
char temperatureString[50];
char humidityString[50];
char pressureString[50];
char altitudeString[50];
// DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;  
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
float w;
char waterString[50];

// NTP
#include <NTPClient.h>
#include <WiFiUdp.h>
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String epochTime;
String formattedDate;
String dayStamp;
String timeStamp;

/******************************************************************************/

void setup()
{

  Serial.begin(baud);

  connectAWS();

  timeClient.begin();

  bme.begin(bme_address);
  sensors.begin();

  delay(100);
}

void loop()
{

  now = millis();
  // Publishes new temperature and humidity every period
  if (now - lastMeasure > period)
  {
    lastMeasure = now;
    // Get NTP (Network Time Protocol)
    ntpValues();
    // Collects all the data first
    bmeValues();
    ds18b20Value();
    // Publishes the data on serial
    serials();
    // Publishes all the data via MQTT
    publishMessage();
    mqttclient.loop();
    delay(1000);
  }
}

/******************************************************************************/

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("WiFi connected. IP: ");
  Serial.println(WiFi.localIP());

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  mqttclient.begin(ENDPOINT, 8883, net);
  // Create a message handler
  mqttclient.onMessage(messageHandler);
  // Connecting to AWS IOT
  Serial.println();
  Serial.println("Connecting to AWS IOT");

  while (!mqttclient.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }

  if (!mqttclient.connected())
  {
    Serial.println("AWS connection timed out!");
    return;
  }

  // Subscribe to a topic and test MQTT Connection
  mqttclient.subscribe(ECONOBOT_CONNECTION);
  Serial.println("Connected to AWS. Sending MQTT test message");
  mqttclient.publish(ECONOBOT_CONNECTION, "EconobotDemo Connected!");
  Serial.println();
}

void publishMessage()
{
  Serial.println();
  Serial.print("Sending MQTT to ");
  Serial.println(ECONOBOT_DATA);
  StaticJsonDocument<200> doc;
  doc["timestamp"] = epochTime;
  doc["date"] = dayStamp;
  doc["time"] = timeStamp;
  doc["temperature"] = t;
  doc["pressure"] = p;
  doc["humidity"] = h;
  doc["waterTemperature"] = w;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  mqttclient.publish(ECONOBOT_DATA, jsonBuffer);
  delay(500);
  Serial.println("Message sent");
}

// For when you receive a message
void messageHandler(String &topic, String &payload)
{
  Serial.println();
  Serial.println("incoming: " + topic + " - " + payload);
  Serial.println();
}

/******************************************************************************/

void ntpValues()
{

  while (!timeClient.update())
  {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  epochTime = timeClient.getEpochTime();
  formattedDate = timeClient.getFormattedDate();
  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  // Extract time
  timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
}

void bmeValues()
{

  t = bme.readTemperature(); // In C
  dtostrf(t, 5, 2, temperatureString);
  h = bme.readHumidity(); // In %
  dtostrf(h, 5, 2, humidityString);
  p = bme.readPressure() / 100.0F; // In hPa
  dtostrf(p, 6, 2, pressureString);

  if (isnan(t) || isnan(h) || isnan(p))
  {
    Serial.println("Failed to read from BME sensor!");
    return;
  }
}

void ds18b20Value() {
  sensors.requestTemperatures(); 
  w = sensors.getTempCByIndex(0);
  dtostrf(w, 6, 2, waterString);
}

void serials()
{

  ++value;
  snprintf(msg, 75, "MESSAGE #%ld START", value);
  Serial.println();
  Serial.println(msg);
  Serial.println();
  Serial.println("TIMESTAMP");
  Serial.println(epochTime);
  Serial.println(formattedDate);
  Serial.print("Date: ");
  Serial.println(dayStamp);
  Serial.print("Time: ");
  Serial.println(timeStamp);
  Serial.println();
  Serial.println("WATER");
  Serial.print("Water Temperature (°C): ");
  Serial.println(waterString);
  Serial.println();
  Serial.println("ATMOSPHERE");
  Serial.print("Ambient Temperature (°C): ");
  Serial.println(temperatureString);
  Serial.print("Pressure (hPa): ");
  Serial.println(pressureString);
  Serial.print("Humidity (%): ");
  Serial.println(humidityString);
  Serial.println();
  snprintf(msg, 75, "MESSAGE #%ld END", value);
  Serial.println(msg);
}