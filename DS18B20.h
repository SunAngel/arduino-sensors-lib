//1-wire, DS18B20 temperature sensors
#include <OneWire.h>

//////////////////////////////////////////////////////////////////////
// **************** W1 DS18B20 Temperature sensor ***************** \\
//////////////////////////////////////////////////////////////////////
/*
     _
    - -
   /   \
   |    |
   \___/
    |||
GND, DATA, VCC
*/
OneWire  ds(4);  // on pin 4 (a 4.7K resistor is necessary) // 
byte W1_addr[8];
bool W1_hasNext;
const int16_t W1_badTemp = 1360; // 85 C, looks like W1(or DS18B20) show such temperature on error.. so, we will do same


void initDS18B20() {
  //Send command to all 1-wire devices. We will think, that we have only ds18b20 sensors here.
  ds.reset();
  ds.skip();
  //To initiate a temperature measurement and A-to-D conversion, themaster must issue a Convert T [44h] command. 
  ds.write(0x44);

  //  delay(750);     // 750ms should be enought

  // Foreach sensor....
  ds.reset_search();
  W1_hasNext = true;
}

int16_t getNextDS18B20Temp() {
  byte data[9];

  //float celsius;

  if (ds.search(W1_addr)) {

    if (OneWire::crc8(W1_addr, 7) != W1_addr[7]) {
#ifdef DEBUG
      Serial.println("CRC is not valid!");
#endif
      return W1_badTemp;
    }

    //display.print(W1_addr[7], HEX); // Print las byte of sensor ID to display

    ds.reset();
    ds.select(W1_addr);
    ds.write(0xBE); // Read Scratchpad

    for (int i = 0; i < 9; i++) { // we need 9 bytes
      data[i] = ds.read();
    }

    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time

    //celsius = (float)raw / 16.0;
    return raw;

  } 
  else {
    W1_hasNext = false;
  }
  return W1_badTemp;

}

