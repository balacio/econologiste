/******************************************************************************
  May 1st, 2020

  Debug 9600
    
  6 measures/min

  Topics:   econobotdemo/data - for data sensors
            econobotdemo/data - for communication

  Protocol: AWS IoT + MQTT w/ NTP Timestamp
  Sensors: BME280 + DS18B20 + EZO pH
******************************************************************************/

// AWS and WiFi
#include "secrets.h"
#include "WiFi.h"
#include <WiFiClientSecure.h>
WiFiClientSecure net = WiFiClientSecure();

// MQTT 
#include <MQTTClient.h>
#include <ArduinoJson.h>
#define ECONOBOT_CONNECTION "econobotdemo/connection"
#define ECONOBOT_DATA "econobotdemo/data"
MQTTClient mqttclient = MQTTClient(256);

// Serials
int baud = 9600;
char msg[50];
int value = 0;

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;
int period = 10 * 1000; //interval measure in sec

// NTP
#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String epochTime;
String formattedDate;
String dayStamp;
String timeStamp;

// BME280 and EZO pH i2c connections
#include <Wire.h>

// BME280
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
const int oneWireBus = 4;
OneWire oneWire(oneWireBus);
DallasTemperature sensor_water(&oneWire);
float w;
char waterString[50];

// EZO pH
#define ph_address 99
byte code = 0;                        //used to hold the I2C response code.
char ph_data[20];                    //we make a 20-byte character array to hold incoming data from the pH circuit.
byte in_char = 0;                     //used as a 1 byte buffer to store inbound bytes from the pH Circuit.
byte i = 0;                           //counter used for ph_data array.
int time_wire = 900;                      //used to change the delay needed depending on the command sent to the EZO Class pH Circuit.
float ph_float;                       //float var used to hold the float value of the pH.



/******************************************************************************/

void setup()
{

    Serial.begin(baud);

    connectAWS();

    timeClient.begin();

    bme.begin(bme_address);
    sensor_water.begin();
    Wire.begin();

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
        phValues();
        delay(1000); 
        // Publishes the data on serial
        serials();
        // Publishes and receives all the data via MQTT
        publishMessage();
        mqttclient.loop();
        
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

/******************************************************************************/

void ntpValues()
{

    while (!timeClient.update())
    {
        timeClient.forceUpdate();
    }
    epochTime = timeClient.getEpochTime();
    formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
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
        Serial.println("Failed to read from BME sensor");
        return;
    }
}

void ds18b20Value()
{
    sensor_water.requestTemperatures();
    w = sensor_water.getTempCByIndex(0);
    dtostrf(w, 6, 2, waterString);
}

void phValues() {

  Wire.beginTransmission(ph_address);    //call the circuit by its ID number.
  Wire.write('r');                    //hard code r to read continually
  Wire.endTransmission();             //end the I2C data transmission.
  delay(time_wire);                       //wait the correct amount of time for the circuit to complete its instruction.

  Wire.requestFrom(ph_address, 20, 1);   //call the circuit and request 20 bytes (this may be more than we need)
  code = Wire.read();                 //the first byte is the response code, we read this separately.

  switch (code) {                   //switch case based on what the response code is.
    //        case 1:                         //decimal 1.
    //        Serial.println("Success");    //means the command was successful.
    //        break;                        //exits the switch case.

    case 2:                                 //decimal 2.
      Serial.println("pH Data Failed");     //means the command has failed.
      break;                                //exits the switch case.

    case 254:                       //decimal 254.
      Serial.println("pH Data Pending");    //means the command has not yet been finished calculating.
      break;                        //exits the switch case.

    case 255:                       //decimal 255.
      Serial.println("No pH Data");    //means there is no further data to send.
      break;                        //exits the switch case.
  }

  while (Wire.available()) {        //are there bytes to receive.
    in_char = Wire.read();          //receive a byte.
    ph_data[i] = in_char;          //load this byte into our array.
    i += 1;                         //incur the counter for the array element.
    if (in_char == 0) {             //if we see that we have been sent a null command.
      i = 0;                        //reset the counter i to 0.
      Wire.endTransmission();       //end the I2C data transmission.
      break;                        //exit the while loop.
    }
  }
  ph_float=atof(ph_data);
}

void serials()
{

    ++value;
    snprintf(msg, 75, "SCREEN MESSAGE #%ld START", value);
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
    Serial.print("pH: ");
    Serial.println(ph_data);
    Serial.print("pH (float): ");
    Serial.println(ph_float);
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
    Serial.print("Sending MQTT to ");
    publishMessage;
    delay(500);
    Serial.println("Message sent");
    Serial.println();
    snprintf(msg, 75, "SCREEN MESSAGE #%ld END", value);
    Serial.println(msg);
}

void publishMessage()
{
    Serial.println();
    Serial.print("Publishing to: ");
    Serial.println(ECONOBOT_DATA);
    StaticJsonDocument<200> doc;
    doc["timestamp"] = epochTime;
    doc["date"] = dayStamp;
    doc["time"] = timeStamp;
    doc["temperature"] = t;
    doc["pressure"] = p;
    doc["humidity"] = h;
    doc["waterTemperature"] = w;
    doc["pH"] = ph_float;
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
