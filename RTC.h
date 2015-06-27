#include <Wire.h>
#define DS3231_ADDRESS 0x68
//http://datasheets.maximintegrated.com/en/ds/DS3231.pdf


void initRTCAlarm() {
  // Get RTC control and status registers
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((uint8_t)0x0E); //Control register
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDRESS, 2);
  uint8_t rtcConfig = Wire.read();
  uint8_t rtcStatus = Wire.read();

  // Set RTC Alarm 2 to alarm every minute
  Wire.beginTransmission(DS3231_ADDRESS);
  // Set Alarm 2 to each minute
  Wire.write((uint8_t)0x0B); //Alarm 2
  Wire.write((uint8_t)0b10000000); //Set A2M2 // A2M2 & Minutes
  Wire.write((uint8_t)0b10000000); //Set A2M3 // A2M3 & 12/24 & Hour
  Wire.write((uint8_t)0b10000000); //Set A2M4 // A2M4 & DY/DT & Day

  rtcConfig &= ~0b11000011; // Clear both Alarms, just in case and ~EOSC & BBSQW
  rtcConfig |=  0b00000110; // Set Alarm 2 and interrupts
  Wire.write(rtcConfig); // Set Control register

  rtcStatus &= ~0b00001011; // Clear both Alarms flags and EN32kHz
  Wire.write(rtcStatus);
  Wire.endTransmission();
}

void clearRTCAlarmBit() {
  // Reset RTC Alarm flags
  //detachInterrupt(0);
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((uint8_t)0x0F); //Control/Status register
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDRESS, 1);
  uint8_t rtcStatus = Wire.read();

  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((uint8_t)0x0F); //Control/Status register
  rtcStatus &= ~0b00000011; // Clear both Alarm flags
  Wire.write(rtcStatus); // Set Control register
  Wire.endTransmission();
}

uint8_t RTCbcdToDec(uint8_t val)  {
  // Convert binary coded decimal to normal decimal numbers.
  return ((val >> 4) * 10) + (val % 16);
}

uint8_t RTCDecTobcd(uint8_t val)  {
  // Convert normal decimal numbers to binary coded decimal.
  return (val % 10) + ((val / 10) << 4);
}

void setRTCTime(uint8_t dow, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t sec) {
  // Get RTC control and status registers
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((uint8_t)0x00); //Seconds register

  Wire.write((uint8_t) RTCDecTobcd(sec)    & 0b01111111); //0 + seconds
  Wire.write((uint8_t) RTCDecTobcd(minute) & 0b01111111); //0 + minutes
  Wire.write((uint8_t) RTCDecTobcd(hour)   & 0b00111111); //0 + 12/~24 + hours

  Wire.write((uint8_t) RTCDecTobcd(dow)    & 0b00000111); //0x5 + DoW

  Wire.write((uint8_t) RTCDecTobcd(day)    & 0b00111111); //0 + 0 + Day
  Wire.write((uint8_t) RTCDecTobcd(month)  & 0b00011111); //Century + 0 + 0 + Month
  Wire.write((uint8_t) RTCDecTobcd(year)); //Year
  Wire.endTransmission();
}

void getRTCTime(uint8_t *dow, uint8_t *year, uint8_t *month, uint8_t *day, uint8_t *hour, uint8_t *minute, uint8_t *sec) {
  // Get RTC control and status registers
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((uint8_t)0x00); //Seconds register
  Wire.endTransmission();

  Wire.requestFrom(DS3231_ADDRESS, 7);
  *sec     = RTCbcdToDec(Wire.read());//0 + seconds
  *minute  = RTCbcdToDec(Wire.read());//0 + minutes
  *hour    = RTCbcdToDec(Wire.read());//0 + 12/~24 + hours

  *dow   = RTCbcdToDec(Wire.read());//0x5 + DoW

  *day   = RTCbcdToDec(Wire.read());//0 + 0 + Day
  *month = RTCbcdToDec(Wire.read() & 0b00011111); //Century + 0 + 0 + Month
  *year  = RTCbcdToDec(Wire.read());//Year
}
