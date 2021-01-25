/*********
  Trial to communicate with pH probe.
  Working. Prints results in Serial Monitor. 
  
  Need to change delay() cmd on ln 70 to a millis() fn
*********/

#include <Wire.h>                     //Enable I2C
#define address 99                 //default I2C ID number for EZO pH Circuit.

byte code = 0;                        //used to hold the I2C response code.
char ph_data[20];                     //we make a 20-byte character array to hold incoming data from the pH circuit.
byte in_char = 0;                     //used as a 1 byte buffer to store inbound bytes from the pH Circuit.
byte i = 0;                           //counter used for ph_data array.
int time_ = 900;                      //used to change the delay needed depending on the command sent to the EZO Class pH Circuit.
int next_op = 5000;                   //used to delay until next measure in ms. 
float ph_float;                       //float var used to hold the float value of the pH.




void setup() {                        //hardware initialization.
  Serial.begin(9600);                 //enable serial port.
  Wire.begin();                       //enable I2C port.
}


void loop() {
  Wire.beginTransmission(address);    //call the circuit by its ID number.
  Wire.write('r');                    //hard code r to read continually
  Wire.endTransmission();             //end the I2C data transmission.
  delay(time_);                       //wait the correct amount of time for the circuit to complete its instruction.

  Wire.requestFrom(address, 20, 1);   //call the circuit and request 20 bytes (this may be more than we need)
  code = Wire.read();                 //the first byte is the response code, we read this separately.

    switch (code) {                   //switch case based on what the response code is.
      case 1:                         //decimal 1.
        Serial.println("Success");    //means the command was successful.
        break;                        //exits the switch case.

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
      ph_data[i] = in_char;           //load this byte into our array.
      i += 1;                         //incur the counter for the array element.
      if (in_char == 0) {             //if we see that we have been sent a null command.
        i = 0;                        //reset the counter i to 0.
        Wire.endTransmission();       //end the I2C data transmission.
        break;                        //exit the while loop.
      }
    }

    Serial.println(ph_data);          //print the data.

    delay(next_op);
  
    //Uncomment this section if you want to take the pH value and convert it into floating point number.
    //ph_float=atof(ph_data);
}
