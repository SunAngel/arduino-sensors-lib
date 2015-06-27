#ifndef __VOLTAGEREAD_H
#define __VOLTAGEREAD_H

#ifndef internalVCC
#error "internalVCC must be defined for using voltageRead"
#endif

uint16_t lastVCC = 0;

uint16_t readVcc() {
  //  const float intVCC = 1.069;
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

  delay(75); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  uint16_t vcc = (high << 8) | low;

  //result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  vcc = (internalVCC * 1023 * 1000) / vcc;
  lastVCC = vcc;
  return vcc; // Vcc in millivolts
}

uint16_t getAccurateVCC(uint8_t count = 5, uint8_t delayTime = 10) {
  uint16_t vcc = 0;
  for (uint8_t i = 0; i < count; i++) {
    vcc += readVcc();
    delay(delayTime);
  }
  vcc = vcc / count;
  return vcc;
}


uint16_t accurateAnalogRead(uint8_t pin, uint8_t count = 5, uint8_t delayTime = 10) {
  uint16_t res = 0;
  for (uint8_t i = 0; i < count; i++) {
    res += analogRead(pin);
    delay(delayTime);
  }

  return res / count;
}

uint16_t adcToVCC(uint16_t value, uint16_t vcc = 0) {
  if (vcc == 0) {
    vcc = lastVCC;
  }

  return map(value, 0, 1023, 0, vcc);

}

#endif // __VOLTAGEREAD_H
