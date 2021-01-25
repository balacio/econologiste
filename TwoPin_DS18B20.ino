//
// FILE: TwoPin_DS18B20.ino
// AUTHOR: Rob Tillaart
// VERSION: 0.1.00
// PURPOSE: two pins for two sensors demo
// DATE: 2014-06-13
// URL: http://forum.arduino.cc/index.php?topic=216835.msg1764333#msg1764333
//
// Released to the public domain
//

#include <OneWire.h>
#include <DallasTemperature.h>

#define water_address D3

OneWire oneWire_in(water_address);

DallasTemperature sensor_water(&oneWire_in);

void setup(void)
{
    Serial.begin(9600);

    
}

void loop(void)
{
    sensor_water.begin();
    sensor_water.requestTemperatures();

    Serial.print("Inhouse: ");
    Serial.println(sensor_water.getTempCByIndex(0));
}
