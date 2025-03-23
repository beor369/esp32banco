#ifndef ENCODER_HANDLER_H
#define ENCODER_HANDLER_H

#include <Arduino.h>

void setupEncoder();
extern void displayEncoderPosition(void (*callback)(), void (*callbackindice)(bool), void (*callback_manejar_estado)());

#endif

