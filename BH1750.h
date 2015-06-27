#ifndef __BH1750_H
#define __BH1750_H
//BH1750 Light sensor

/*
* Some of data here was copied from
* https://github.com/claws/BH1750.git
* Also, see
* http://www.elechouse.com/elechouse/images/product/Digital%20light%20Sensor/bh1750fvi-e.pdf
* or
* http://rohmfs.rohm.com/en/products/databook/datasheet/ic/sensor/light/bh1750fvi-e.pdf
*/

#include "Arduino.h"
#include <Wire.h>

// Device states

#define BH1750_Address_L 0x23 // Device address when address pin LOW
#define BH1750_Address_H 0x5C // Device address when address pin LOW

// No active state
#define BH1750_POWER_DOWN 0x00

// Wating for measurment command
#define BH1750_POWER_ON 0x01

// Reset data register value - not accepted in POWER_DOWN mode
#define BH1750_RESET 0x07

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
#define BH1750_CONTINUOUS_HIGH_RES_MODE  0x10

// Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
#define BH1750_CONTINUOUS_HIGH_RES_MODE_2  0x11

// Start measurement at 4lx resolution. Measurement time is approx 16ms.
#define BH1750_CONTINUOUS_LOW_RES_MODE  0x13

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_HIGH_RES_MODE  0x20

// Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_HIGH_RES_MODE_2  0x21

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_LOW_RES_MODE  0x23

// Global variables is not very good, but anyway.
uint8_t bh1750Addr;

uint8_t bh1750_mode = 1;

#define BH1750_DEFAULT_DELAY B01000101
// b01000101 means 69
#define BH1750_MIN_DELAY 31
#define BH1750_MAX_DELAY 254

uint8_t bh1750_delay = BH1750_DEFAULT_DELAY;

void BH1750_setMode(uint8_t mode) {
  //Probably I need to do some checks, but i will not
  Wire.beginTransmission(bh1750Addr);
  Wire.write(mode);
  Wire.endTransmission();
}

inline void BH1750_init(uint8_t addr) {
  bh1750Addr = addr;
  BH1750_setMode(BH1750_POWER_DOWN);
}

int32_t BH1750_GetLightIntensity(void) {
  int32_t res = -1;
  
  Wire.requestFrom(bh1750Addr, 2);
  if (Wire.available() >= 2) {
    res = ((Wire.read() << 8) | Wire.read()) / 1.2;
    //I do not know, why i have to divide result by 1.2? but datasheet says so.
  }
  return res;
}


inline uint16_t BH1750_GetLightIntensityOnce(void) {
  BH1750_setMode(BH1750_ONE_TIME_HIGH_RES_MODE);
  delay(180);
  int32_t res = BH1750_GetLightIntensity();
  //BH1750_setMode(BH1750_POWER_DOWN);
  return res;
}

inline float BH1750_CalculateLX(uint16_t value, uint8_t mode, uint8_t bhDelay) {
  return (float) value / (float)mode * ((float)BH1750_DEFAULT_DELAY / (float)bhDelay);

}

inline uint32_t BH1750_CalculateLX_100X(uint16_t value, uint8_t mode, uint8_t bhDelay) {
  return (uint32_t)value * 100 * BH1750_DEFAULT_DELAY / (bhDelay * mode);

}

uint8_t bh1750_delayChanged = 1;
void BH1750_setDelay(uint8_t newDelay) {
  if (bh1750_delay != newDelay) {
    bh1750_delayChanged = 1;
    bh1750_delay = newDelay;
  }
}

void BH1750_adjustDelayAndModeX100(uint32_t lux) {
  // Every value is x100
  if (bh1750_mode == 1) {
    if (lux < 1500000) { // < 15`000.00
      bh1750_mode = 2;
      BH1750_setDelay(BH1750_DEFAULT_DELAY);
    } else if ( (lux >= 1500000) && (lux < 3000000)) { // 15`000.00 - 30`000.00
      BH1750_setDelay(BH1750_DEFAULT_DELAY);
    } else if ( lux >= 3000000) { // > 30`000.00
      BH1750_setDelay(BH1750_MIN_DELAY);
    }
  } else { // mode == 2
    if (lux >= 1500000) { // > 15`000.00
      bh1750_mode = 1;
      BH1750_setDelay(BH1750_DEFAULT_DELAY);
    } else if ( (lux < 1500000) && (lux > 500000)) { // 5`000.00 - 15`000.00
      BH1750_setDelay(BH1750_DEFAULT_DELAY);
    } else if ( lux == 0) { // if lux == 0, then there is posibility, that there is too much light.
    bh1750_mode = 1;
    BH1750_setDelay(BH1750_MIN_DELAY);
    // But if there will be 0lx after next measurement, then sensor will return to mode 2.
    } else if ( lux <= 500000 ) { // < 5`000.00
      BH1750_setDelay(BH1750_MAX_DELAY);
    } 
  }

  if (bh1750_delayChanged == 1) { // Check if delay was changed to do not do it every time
    BH1750_setMode(BH1750_POWER_ON);
    BH1750_setMode(B01000000 | (bh1750_delay >> 5)); // Write High MTReg bits (7,6,5 bits)
    BH1750_setMode(B01100000 | (bh1750_delay & B011111)); // Write Low MTReg bits (4,3,2,1,0 bits)
    BH1750_setMode(BH1750_POWER_DOWN);
    bh1750_delayChanged = 0;
  }
}

inline void BH1750_adjustDelayAndMode(uint32_t lux) {
  BH1750_adjustDelayAndModeX100(lux * 100);
}

#endif //__BH1750
