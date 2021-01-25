/*************************************************************
  Oct 9th, 2018

  BLYNK + MQTT:
  ESP8266 + BH1750 + DS18B20  + EZO (+ SONOFF)

  "int period" 12 measures/min
  Print in Debugger (Serials)

  Debug @ 9,600
    Prints sensors results

  Topics: Temperature, Humidity, Pressure, Altitude
          pH
          WaterTemperature
          Lux
 *************************************************************/



/***** Wireless connection *****/
#include <ESP8266WiFi.h>          // WiFi
#include <PubSubClient.h>         // MQTT
WiFiClient espClient;
PubSubClient client(espClient);


/***** Physical connection *****/
#include <Wire.h>                 // I2C
#include <OneWire.h>              // one pin


/***** Blynk *****/
#include <BlynkSimpleEsp8266.h>


/***** WiFi + Blynk credentials *****/
char ssid[] = "69";
char pass[] = "3102797029";
char auth[] = "af8188010ec14cfa8473aa19f0e1a5d0";
const char* mqtt_server = "192.168.0.19";


/***** Loop intervals *****/
long lastMsg = 0;
int period = 5 * 1000;            // In ms


/***** BH1750 *****/
#include <BH1750.h>
#define bh_address 0x23
BH1750 lightMeter;
float lux;
char luxString[50];


/***** DS18B20 *****/
#include <DallasTemperature.h>
#define water_address D3
OneWire oneWire_in(water_address);
DallasTemperature sensor_water(&oneWire_in);
float w;
char waterString[50];


/***** pH *****/
#define ph_address 99
byte code = 0;                        //used to hold the I2C response code.
char phString[20];                    //we make a 20-byte character array to hold incoming data from the pH circuit.
byte in_char = 0;                     //used as a 1 byte buffer to store inbound bytes from the pH Circuit.
byte i = 0;                           //counter used for phString array.
int time_ = 900;                      //used to change the delay needed depending on the command sent to the EZO Class pH Circuit.
float ph_float;                       //float var used to hold the float value of the pH.



/*************************************************************/
void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}




void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String payloadTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadTemp += (char)payload[i];
  }

  /*
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
  */
}




void reconnect() {

  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // ... and resubscribe
      //clientSubscribe();
      //client.subscribe("lamp");
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
  reconnect();
  Blynk.run();

  long now = millis();
  if (now - lastMsg > period) {
    lastMsg = now;

    BH1750values();
    DS18B20values();
    phValues();
  }
}




void BH1750values() {
  Wire.begin();
  lightMeter.begin();
  lux = lightMeter.readLightLevel();            // In lux
  dtostrf(lux, 6, 2, luxString);

  Blynk.virtualWrite(V1, luxString);
  client.publish("lux", luxString);
}



void DS18B20values() {
  sensor_water.begin();
  sensor_water.requestTemperatures();
  w = sensor_water.getTempCByIndex(0);          // In celsius
  dtostrf(w, 6, 2, waterString);

  Blynk.virtualWrite(V2, waterString);
  client.publish("water_temperature", waterString);
}



void phValues() {

  Wire.beginTransmission(ph_address);           //call the circuit by its ID number.
  Wire.write('r');                              //hard code r to read continually
  Wire.endTransmission();                       //end the I2C data transmission.
  delay(time_);                                 //wait the correct amount of time for the circuit to complete its instruction.

  Wire.requestFrom(ph_address, 20, 1);          //call the circuit and request 20 bytes (this may be more than we need)
  code = Wire.read();                           //the first byte is the response code, we read this separately.

  switch (code) {                               //switch case based on what the response code is.
    //      case 1:                             //decimal 1.
    //        Serial.println("Success");        //means the command was successful.
    //        break;                            //exits the switch case.

    case 2:                                     //decimal 2.
      Serial.println("Failed");                 //means the command has failed.
      break;                                    //exits the switch case.

    case 254:                                   //decimal 254.
      Serial.println("Pending");                //means the command has not yet been finished calculating.
      break;                                    //exits the switch case.

    case 255:                                   //decimal 255.
      Serial.println("No Data");                //means there is no further data to send.
      break;                                    //exits the switch case.
  }

  while (Wire.available()) {                    //are there bytes to receive.
    in_char = Wire.read();                      //receive a byte.
    phString[i] = in_char;                      //load this byte into our array.
    i += 1;                                     //incur the counter for the array element.
    if (in_char == 0) {                         //if we see that we have been sent a null command.
      i = 0;                                    //reset the counter i to 0.
      Wire.endTransmission();                   //end the I2C data transmission.
      break;                                    //exit the while loop.
    }
  }
  Blynk.virtualWrite(V3, phString);
  client.publish("ph", phString);
}
