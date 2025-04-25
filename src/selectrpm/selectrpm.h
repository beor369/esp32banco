#ifndef SELECTRPM
#define SELECTRPM
#include <Arduino.h>
#include "config.h"

void setupselectrpm();
void loopselectrpm();
void updateEncoderParameters();
void displayMenu();
void simularRPM(unsigned int rpm, unsigned long testDuration, unsigned long pulseTime);

enum State
{
  SELECT_RPM,
  SELECT_PULSE,
  SELECT_TIME,
  RUN_TEST
};
extern State currentState;
// Valores base iniciales
extern long rpmValue ;       // Base 1000 RPM
extern long pulseWidthValue;  // Base 10 ms
extern long testTimeValue; // Base 10,000 ms (10 s)
extern unsigned long lastButtonPress;
#endif  