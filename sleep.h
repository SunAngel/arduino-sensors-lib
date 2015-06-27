//Sleep module
#ifndef SA_SLEEP_H_
#define SA_SLEEP_H_
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

//volatile uint8_t watchdogActivated = 1;

// Define watchdog timer interrupt.
/**ISR(WDT_vect)
{
  // Set the watchdog activated flag.
  // Note that you shouldn't do much work inside an interrupt handler.
  watchdogActivated++;
}*/

// Put the Arduino to sleep.
void sleep()
{
  /*
   * The 5 different modes are:
   *     SLEEP_MODE_IDLE         -the least power savings
   *     SLEEP_MODE_ADC
   *     SLEEP_MODE_PWR_SAVE
   *     SLEEP_MODE_STANDBY
   *     SLEEP_MODE_PWR_DOWN     -the most power savings
   */
  // Set sleep to full power down.  Only external interrupts or 
  // the watchdog timer can wake the CPU!
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //set_sleep_mode(SLEEP_MODE_ADC);

  // Turn off everything (http://www.nongnu.org/avr-libc/user-manual/group__avr__power.html)
  // https://learn.adafruit.com/low-power-wifi-datalogging/power-down-sleep
  //power_all_disable();

  // Enable sleep and enter sleep mode.
  //sleep_enable();
  //sleep_cpu ();
  /*  sleep_mode() function 
   *  do the same as
   *  sleep_enable();
   *  sleep_cpu();
   *  sleep_disable();
   *
   *  http://www.avrfreaks.net/comment/210481#comment-210481
   *  */
  sleep_mode();

  // CPU is now asleep and program execution completely halts!
  // Once awake, execution will resume at this point.

  // When awake, disable sleep mode 
  //sleep_disable();

  // and turn on all devices.
  power_all_enable();
  /*
   power_adc_enable();
   power_twi_enable();
   power_spi_enable();
   //power_usart0_enable();
   power_timer0_enable();
   //power_timer1_enable();
   //power_timer2_enable();
   */
}

void setupWatchDog() {
  // Setup the watchdog timer to run an interrupt which
  // wakes the Arduino from sleep every 8 seconds.

  // Note that the default behavior of resetting the Arduino
  // with the watchdog will be disabled.

  // This next section of code is timing critical, so interrupts are disabled.
  // See more details of how to change the watchdog in the ATmega328P datasheet
  // around page 50, Watchdog Timer.
  noInterrupts();

  // Set the watchdog reset bit in the MCU status register to 0.
  MCUSR &= ~(1<<WDRF);

  // Set WDCE and WDE bits in the watchdog control register.
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  // Set watchdog clock prescaler bits to a value of 8 seconds.
  WDTCSR = (1<<WDP0) | (1<<WDP3);

  // Enable watchdog as interrupt only (no reset).
  WDTCSR |= (1<<WDIE);

  // Enable interrupts again.
  interrupts();
}


/* 
 lpDelay(quarterSeconds) - Low Power Delay.  Drops the system clock
 to its lowest setting and sleeps for 256*quarterSeconds milliseconds.
 */
int lpDelay(int quarterSeconds) {

  int oldClkPr = CLKPR;  // save old system clock prescale
  CLKPR = 0x80;    // Tell the AtMega we want to change the system clock
  CLKPR = 0x08;    // 1/256 prescaler = 60KHz for a 16MHz crystal
  delay(quarterSeconds);  // since the clock is slowed way down, delay(n) now acts like delay(n*256)
  CLKPR = 0x80;    // Tell the AtMega we want to change the system clock
  CLKPR = oldClkPr;    // Restore old system clock prescale

}

#endif
