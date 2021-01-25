/******************************************************************************
  Oct 5th, 2018

  MQTT + BME280 + DS18B20 + EZO + BH1750
  "int period" 4 measures/min
  Code to switch on the builtin LED

  Debug @ 115,200
    Prints sensors results

  Topics: inTopic (LED switch),
          Temperature, Humidity, Pressure, Altitude
          pH
          WaterTemperature
          Lux

******************************************************************************/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/*****/
#include <Wire.h>                 // I2C
#include <OneWire.h>              // one pin
//BME280
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
//DS18B20
#include <DallasTemperature.h>
//BH1750
#include <BH1750.h>
BH1750 lightMeter;
#define bh_address 0x23
//BME280
Adafruit_BME280 bme;              // I2C
#define bme_address 0x76
#define SEALEVELPRESSURE_HPA (1013.25)
//EZO
#define ph_address 99
//DS18B20
#define water_address D3
//Red LED
#define lamp D4
int luxthreshold = 1000;
/*****/

#define BUILTIN_LED 2

const char* ssid = "69";
const char* password = "3102797029";
const char* mqtt_server = "192.168.0.19";

/*****/
//Loop
int period = 15 * 1000;             //interval measure in sec
//BME280
float h, t, p, a;
char temperatureString[50];
char humidityString[50];
char pressureString[50];
char altitudeString[50];
//EZO
byte code = 0;                        //used to hold the I2C response code.
char phString[20];                    //we make a 20-byte character array to hold incoming data from the pH circuit.
byte in_char = 0;                     //used as a 1 byte buffer to store inbound bytes from the pH Circuit.
byte i = 0;                           //counter used for phString array.
int time_ = 900;                      //used to change the delay needed depending on the command sent to the EZO Class pH Circuit.
float ph_float;                       //float var used to hold the float value of the pH.
//DS18B20
float w;
char waterString[50];
//BH1750
float lux;
char luxString[50];
//Serials
char msg[50];
int value = 0;
/*****/

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

/*****/
//BH1750

OneWire oneWire_in(water_address);
DallasTemperature sensor_water(&oneWire_in);
/*****/

void setup() {

  /*****/
  //LED
  pinMode(lamp, OUTPUT);     // Initialize the LED pin as an output
//  digitalWrite(lamp, LOW);  //Initialize the builtin led as off
  /*****/
  //  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  //  digitalWrite(BUILTIN_LED, HIGH);  //Initialize the builtin led as off
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

  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  //  for (int i = 0; i < length; i++) {
  //    Serial.println((char)payload[i]);
  //  }
  //  Serial.println();
  //
  //  // Switch on the LED if an 1 was received as first character
  //  if ((char)payload[0] == 'on') {
  //    digitalWrite(lamp, LOW);   // Turn the LED on (Note that LOW is the voltage level
  //    // but actually the LED is on; this is because
  //    // it is acive low on the ESP-01)
  //  } else {
  //    digitalWrite(lamp, HIGH);  // Turn the LED off by making the voltage HIGH
  //  }
  String payloadTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadTemp += (char)payload[i];
  }
  Serial.println();

  Serial.print("payloadTemp: ");
  Serial.println(payloadTemp);

  if (topic == "lamp") {
    Serial.print("Changing lamp to ");
    if (payloadTemp == "on") {
      digitalWrite(lamp, HIGH);
      Serial.print("On");
    }
    else if (payloadTemp == "off") {
      digitalWrite(lamp, LOW);
      Serial.print("Off");
    }
  }
  Serial.println();
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
      clientSubscribe();
      //      client.subscribe("inTopic");
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
  if (now - lastMsg > period) {
    lastMsg = now;
    /*****/
    bmeValues();
    DS18B20values();
    BH1750values();
    phValues();
    clientPublish();
    //clientSubscribe();
    serials();
    /*****/
  }
}




/*****/
void clientPublish() {

  client.publish("temperature", temperatureString);
  client.publish("pressure", pressureString);
  client.publish("humidity", humidityString);
  client.publish("lux", luxString);
  client.publish("ph", ph);
  client.publish("water_temperature", waterString);
}





void clientSubscribe() {
  client.subscribe("lamp");
}





void serials() {

  ++value;
  snprintf (msg, 75, "MESSAGE #%ld START", value);
  Serial.println();
  Serial.println(msg);
  Serial.println();
  Serial.println("ATMOSPHERE");
  Serial.print("Ambient Temperature (°C): ");
  Serial.println(temperatureString);
  Serial.print("Pressure (hPa): ");
  Serial.println(pressureString);
  Serial.print("Humidity (%): ");
  Serial.println(humidityString);
  Serial.print("Intensity (lux): ");
  Serial.println(luxString);
  Serial.println();
  Serial.println("WATER");
  Serial.print("pH: ");
  Serial.println(phString);
  Serial.print("Water Temperature (°C): ");
  Serial.println(waterString);
  snprintf (msg, 75, "MESSAGE #%ld END", value);
  Serial.println();
  Serial.println(msg);
}





void bmeValues() {

  bme.begin(bme_address);
  t = bme.readTemperature();                        // In C
  dtostrf(t, 5, 2, temperatureString);
  h = bme.readHumidity();                           // In %
  dtostrf(h, 5, 2, humidityString);
  p = bme.readPressure() / 100.0F;                  // In hPa
  dtostrf(p, 6, 2, pressureString);
}





void DS18B20values () {

  sensor_water.begin();
  sensor_water.requestTemperatures();
  w = sensor_water.getTempCByIndex(0);                  // In celsius
  dtostrf(w, 6, 2, waterString);
}





void BH1750values () {
  Wire.begin();
  lightMeter.begin();
  lux = lightMeter.readLightLevel();                  // In lux
  dtostrf(lux, 6, 2, luxString);
}





//LEAVE IT AT THE END, TAKES 900ms TO RUN
void phValues() {

  Wire.beginTransmission(ph_address);    //call the circuit by its ID number.
  Wire.write('r');                    //hard code r to read continually
  Wire.endTransmission();             //end the I2C data transmission.
  delay(time_);                       //wait the correct amount of time for the circuit to complete its instruction.

  Wire.requestFrom(ph_address, 20, 1);   //call the circuit and request 20 bytes (this may be more than we need)
  code = Wire.read();                 //the first byte is the response code, we read this separately.

  switch (code) {                   //switch case based on what the response code is.
    //      case 1:                         //decimal 1.
    //        Serial.println("Success");    //means the command was successful.
    //        break;                        //exits the switch case.

    case 2:                         //decimal 2.
      Serial.println("Failed");     //means the command has failed.
      break;                        //exits the switch case.

    case 254:                       //decimal 254.
      Serial.println("Pending");    //means the command has not yet been finished calculating.
      break;                        //exits the switch case.

    case 255:                       //decimal 255.
      Serial.println("No Data");    //means there is no further data to send.
      break;                        //exits the switch case.
  }

  while (Wire.available()) {        //are there bytes to receive.
    in_char = Wire.read();          //receive a byte.
    phString[i] = in_char;          //load this byte into our array.
    i += 1;                         //incur the counter for the array element.
    if (in_char == 0) {             //if we see that we have been sent a null command.
      i = 0;                        //reset the counter i to 0.
      Wire.endTransmission();       //end the I2C data transmission.
      break;                        //exit the while loop.
    }
  }
}
/*****/
