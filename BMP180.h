#ifndef _BMP180_H
#define _BMP180_H
//I2C BMC180 (Pressure sensor)
#include <Wire.h>

//////////////////////////////////////////////////////////////////////
// *************** Bosh BMC180 i2c pressure sensor **************** \\
//////////////////////////////////////////////////////////////////////
//Connect the i2c SCL clock pin to your i2c clock pin.
//On the classic Arduino Uno/Duemilanove/Diecimila/etc this is
//Analog pin #5

//Connect the i2c SDA data pin to your i2c data pin.
//On the classic Arduino Uno/Duemilanove/Diecimila/etc this is
//Analog pin #4

#define BMP180_ADDRESS 0x77                 // Sensor address

typedef struct {
  int16_t  ac1, ac2, ac3, b1, b2, mb, mc, md; // Store sensor PROM values from BMP180
  uint16_t ac4, ac5, ac6;                     // Store sensor PROM values from BMP180
} BMP180data_t;

BMP180data_t BMP180data;

// Ultra Low Power       OSS = 0, OSD =  5ms
// Standard              OSS = 1, OSD =  8ms
// High                  OSS = 2, OSD = 14ms
// Ultra High Resolution OSS = 3, OSD = 26ms
const uint8_t BMP_oss = 3;                      // Set oversampling setting
const uint8_t BMP_osd = 26;                     // with corresponding oversampling delay


uint16_t i2c_read_2_bytes(uint8_t code);
uint8_t i2c_read_1_byte(uint8_t code);
int32_t BMP_readPressure();
int32_t BMP_readTemperature();
void initBMP180();
float BMP_getPressure(int32_t b5);

/**********************************************
 * Initialize sensor variables
 **********************************************/
void initBMP180() {

  BMP180data.ac1 = i2c_read_2_bytes(0xAA);
  BMP180data.ac2 = i2c_read_2_bytes(0xAC);
  BMP180data.ac3 = i2c_read_2_bytes(0xAE);
  BMP180data.ac4 = i2c_read_2_bytes(0xB0);
  BMP180data.ac5 = i2c_read_2_bytes(0xB2);
  BMP180data.ac6 = i2c_read_2_bytes(0xB4);
  BMP180data.b1  = i2c_read_2_bytes(0xB6);
  BMP180data.b2  = i2c_read_2_bytes(0xB8);
  BMP180data.mb  = i2c_read_2_bytes(0xBA);
  BMP180data.mc  = i2c_read_2_bytes(0xBC);
  BMP180data.md  = i2c_read_2_bytes(0xBE);
}

/**********************************************
 * Calcualte pressure readings
 **********************************************/
float BMP_getPressure(int32_t b5)
{
  int32_t x1, x2, x3, b3, b6, p, UP;
  uint32_t b4, b7;

  UP = BMP_readPressure(); // Read raw pressure

  b6 = b5 - 4000;
  x1 = (BMP180data.b2 * (b6 * b6 >> 12)) >> 11;
  x2 = BMP180data.ac2 * b6 >> 11;
  x3 = x1 + x2;
  b3 = (((BMP180data.ac1 * 4 + x3) << BMP_oss) + 2) >> 2;
  x1 = BMP180data.ac3 * b6 >> 13;
  x2 = (BMP180data.b1 * (b6 * b6 >> 12)) >> 16;
  x3 = ((x1 + x2) + 2) >> 2;
  b4 = (BMP180data.ac4 * (uint32_t)(x3 + 32768)) >> 15;
  b7 = ((uint32_t)UP - b3) * (50000 >> BMP_oss);
  if (b7 < 0x80000000) {
    p = (b7 << 1) / b4;
  }
  else {
    p = (b7 / b4) << 1;
  } // or p = b7 < 0x80000000 ? (b7 * 2) / b4 : (b7 / b4) * 2;
  x1 = (p >> 8) * (p >> 8);
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * p) >> 16;
  return (p + ((x1 + x2 + 3791) >> 4)); // Return pressure in Pa
}

/**********************************************
 * Read uncompensated temperature
 **********************************************/
int32_t BMP_readTemperature()
{
  int32_t x1, x2, b5, UT;

  Wire.beginTransmission(BMP180_ADDRESS); // Start transmission to device
  Wire.write(0xf4);                       // Sends register address
  Wire.write(0x2e);                       // Write data
  Wire.endTransmission();                 // End transmission
  delay(5);                               // Datasheet suggests 4.5 ms

  UT = i2c_read_2_bytes(0xf6);            // Read uncompensated TEMPERATURE value

  // Calculate true temperature
  x1 = (UT - (int32_t)BMP180data.ac6) * (int32_t)BMP180data.ac5 >> 15;
  x2 = ((int32_t)(BMP180data.mc) << 11) / (x1 + (int32_t)(BMP180data.md));
  b5 = x1 + x2;
  return b5;
}

/**********************************************
 * Read uncompensated pressure value
 **********************************************/
int32_t BMP_readPressure()
{
  int32_t value;
  Wire.beginTransmission(BMP180_ADDRESS);   // Start transmission to device
  Wire.write(0xf4);                         // Sends register address to read from
  Wire.write(0x34 + (BMP_oss << 6));        // Write data
  Wire.endTransmission();                   // SEd transmission
  delay(BMP_osd);                           // Oversampling setting delay
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(0xf6);                         // Register to read
  Wire.endTransmission();
  Wire.requestFrom(BMP180_ADDRESS, 3);      // Request three bytes
  if (Wire.available() >= 3)
  {
    value = (((int32_t)Wire.read() << 16) | ((int32_t)Wire.read() << 8) | ((int32_t)Wire.read())) >> (8 - BMP_oss);
  }
  return value;                             // Return value
}

/**********************************************
 * Read 1 byte from the BMP sensor
 **********************************************/
uint8_t i2c_read_1_byte(uint8_t code)
{
  uint8_t value;
  Wire.beginTransmission(BMP180_ADDRESS);         // Start transmission to device
  Wire.write(code);                               // Sends register address to read from
  Wire.endTransmission();                         // End transmission
  Wire.requestFrom(BMP180_ADDRESS, 1);            // Request data for 1 byte to be read
  if (Wire.available() >= 1)
  {
    value = Wire.read();                          // Get 1 byte of data
  }
  return value;                                   // Return value
}

/**********************************************
 * Read 2 bytes from the BMP sensor
 **********************************************/
uint16_t i2c_read_2_bytes(uint8_t code)
{
  uint16_t value;
  Wire.beginTransmission(BMP180_ADDRESS);         // Start transmission to device
  Wire.write(code);                               // Sends register address to read from
  Wire.endTransmission();                         // End transmission
  Wire.requestFrom(BMP180_ADDRESS, 2);            // Request 2 bytes from device
  if (Wire.available() >= 2)
  {
    value = (Wire.read() << 8) | Wire.read();     // Get 2 bytes of data
  }
  return value;                                   // Return value
}


#endif
