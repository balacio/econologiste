/******************************************************************************
  Oct 5th, 2018

  MQTT + BME280 + DS18B20 + EZO + BH1750 + LED
  "int period" 4 measures/min

  Debug @ 115,200
    Prints sensors results

  Topics: lamp
          Temperature, Humidity, Pressure, Altitude
          pH
          WaterTemperature
          lux

******************************************************************************/

//#include <ESP8266WiFi.h>
//#include <PubSubClient.h>

#include <Wire.h>
#include <OneWire.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;
#define bme_address 0x76
#define SEALEVELPRESSURE_HPA (1013.25)

#include <DallasTemperature.h>
#define water_address D3

#include <BH1750.h>
BH1750 lightMeter;
#define bh_address 0x23

#define ph_address 99

#define builtinled 2
#define lamp D4

/***** Network *****/
const char* ssid = "69";
const char* password = "3102797029";
const char* mqtt_server = "192.168.0.19";

// Initializes the espClient. You should change the espClient name if you have
// multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;
int period = 15 * 1000;             //interval measure in sec

/*****/
//MQTT
String pltemp;
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

OneWire oneWire_in(water_address);
DallasTemperature sensor_water(&oneWire_in);





// Don't change the function below.
// This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);
  // Connect to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected. IP: ");
  Serial.println(WiFi.localIP());
}





// This functions is executed when some device publishes a message to a topic
// that your ESP8266 is subscribed to change the function below to add logic
// to your program, so when a device publishes a message to a topic that
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  Serial.print("Message: ");
  String payload;
  payload = pltemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    payload += (char)message[i];
  }
  Serial.println();

  // If a message is received on the topic room/lamp, you check if the message
  // is either on or off. Turns the lamp GPIO according to the message
  if (topic == "lamp") {
    Serial.print("Lamp status: ");
    if (payload == "on") {
      digitalWrite(lamp, HIGH);
      Serial.print(payload);
    }
    else if (payload == "off") {
      digitalWrite(lamp, LOW);
      Serial.print(payload);
    }
  }

  if (topic == "builtin") {
    Serial.print("Builtin LED status: ");
    if (payload == "on") {
      digitalWrite(builtinled, HIGH);
      Serial.print(payload);
    }
    else if (payload == "off") {
      digitalWrite(builtinled, LOW);
      Serial.print(payload);
    }
  }

  Serial.println();
}





// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to
// more topics with your ESP8266
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    /*
      YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS
      WITH MQTT MULTIPLE CONNECTIONS
      To change the ESP device ID, you will have to give a new name to the ESP8266.
      Here's how it looks:
       if (client.connect("ESP8266Client")) {
      You can do it like this:
       if (client.connect("ESP1_Office")) {
      Then, for the other ESP:
       if (client.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      clientSubscribe();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}





void clientSubscribe() {
  client.subscribe("lamp");
  client.subscribe("builtin");
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial
// communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void setup() {
  pinMode(lamp, OUTPUT);
  digitalWrite(lamp, LOW);

  pinMode(builtinled, OUTPUT);

  bme.begin(bme_address);
  sensor_water.begin();
  Wire.begin();
  lightMeter.begin();

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}





// For this project, you don't need to change anything in the loop function.
// Basically it ensures that you ESP is connected to your broker
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if (!client.loop())
    client.connect("ESP8266Client");

  now = millis();
  // Publishes new temperature and humidity every 30 seconds
  if (now - lastMeasure > period) {
    lastMeasure = now;

    bmeValues();
    DS18B20values();
    BH1750values();
    phValues();
    clientPublish();
    serials();
  }
}





void clientPublish() {

  client.publish("Temperature", temperatureString);
  client.publish("Pressure", pressureString);
  client.publish("Humidity", humidityString);
  client.publish("lux", luxString);
  client.publish("pH", phString);
  client.publish("WaterTemperature", waterString);
}





void serials() {

  ++value;
  snprintf (msg, 75, "MESSAGE #%ld START", value);
  Serial.println();
  Serial.println(msg);
  Serial.println();
  Serial.println("WATER");
  Serial.print("pH: ");
  Serial.println(phString);
  Serial.print("Water Temperature (°C): ");
  Serial.println(waterString);
  Serial.println(); Serial.println("ATMOSPHERE");
  Serial.print("Ambient Temperature (°C): ");
  Serial.println(temperatureString);
  Serial.print("Pressure (hPa): ");
  Serial.println(pressureString);
  Serial.print("Humidity (%): ");
  Serial.println(humidityString);
  Serial.print("Intensity (lux): ");
  Serial.println(luxString);
  Serial.print("Lamp status: ");
  Serial.print(pltemp);
  //Serial.println("Go Figure!");
  Serial.println();
  snprintf (msg, 75, "MESSAGE #%ld END", value);
  Serial.println(msg);
}

void bmeValues() {

  t = bme.readTemperature();                        // In C
  dtostrf(t, 5, 2, temperatureString);
  h = bme.readHumidity();                           // In %
  dtostrf(h, 5, 2, humidityString);
  p = bme.readPressure() / 100.0F;                  // In hPa
  dtostrf(p, 6, 2, pressureString);

  if (isnan(t) || isnan(h) || isnan(p)) {
    Serial.println("Failed to read from BME sensor!");
    return;
  }
}

void DS18B20values () {

  sensor_water.requestTemperatures();
  p = sensor_water.getTempCByIndex(0);                  // In celsius
  dtostrf(p, 6, 2, waterString);

  if (isnan(p)) {
    Serial.println("Failed to read from Water Temp sensor!");
    return;
  }
}


void BH1750values () {

  lux = lightMeter.readLightLevel();                  // In lux
  dtostrf(lux, 6, 2, luxString);

  if (isnan(lux)) {
    Serial.println("Failed to read from BH1750 sensor!");
    return;
  }
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
