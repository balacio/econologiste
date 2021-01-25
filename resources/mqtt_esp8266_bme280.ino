/**************************************************************************
  Sep 30th, 2018

  MQTT + BME280

  Topics: inTopic (LED switch), Temperature, Humidity, Pressure, Altitude
**************************************************************************/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/*****/
#include <Wire.h>                 // I2C

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme; // I2C
#define bme_address 0x76
#define SEALEVELPRESSURE_HPA (1013.25)
/*****/

#define BUILTIN_LED 2

const char* ssid = "69";
const char* password = "3102797029";
const char* mqtt_server = "192.168.0.19";

/*****/
float h, t, p, a;
char temperatureString[50];
char humidityString[50];
char pressureString[50];
char altitudeString[50];
/*****/

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
//char msg[50];
//int value = 0;





void setup() {
  
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(BUILTIN_LED, HIGH);  //Initialize the builtin led as off
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}





void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}





void callback(char* topic, byte* payload, unsigned int length) {
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}





void reconnect() {
  
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}





void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    //    ++value;
    //    snprintf (msg, 75, "hello world #%ld", value);
    //    Serial.print("Publish message: ");
    //    Serial.println(msg);
    //    client.publish("outTopic", msg);
    /*****/
    bmeValues();
    /*****/
  }
}





void bmeValues() {
  
  bme.begin(bme_address);
  
  t = bme.readTemperature();                        // In C
  dtostrf(t, 5, 0, temperatureString);
  client.publish("Temperature", temperatureString);
  
  h = bme.readHumidity();                           // In %
  dtostrf(h, 5, 0, humidityString);
  client.publish("Humidity", humidityString);
  
  p = bme.readPressure() / 100.0F;                  // In hPa
  dtostrf(p, 6, 0, pressureString);
  client.publish("Pressure", pressureString);
  
  a = bme.readAltitude(SEALEVELPRESSURE_HPA);       // In m
  dtostrf(a, 6, 0, altitudeString);
  client.publish("Altitude", altitudeString);

}
