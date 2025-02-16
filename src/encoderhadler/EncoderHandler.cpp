#include "EncoderHandler.h"
#include "Config.h"
#include "menu/Menu.h"  // Para actualizar índices y estados

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern ESP32Encoder encoder;
extern unsigned long lastEncoderMoveTime;

// Configurar pines y el encoder
void setupEncoder() {
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  encoder.attachHalfQuad(ENCODER_PIN_A, ENCODER_PIN_B);
  encoder.clearCount();
  pinMode(botonPin, INPUT_PULLUP);
}

// Manejo del movimiento del encoder y actualización de índices según el estado
void displayEncoderPosition() {
  int valorEncoder = encoder.getCount();
  if (millis() - lastEncoderMoveTime > encoderMoveDelay) {
    if (valorEncoder > 0) {
      actualizarIndice(true);  // Función definida en Menu.cpp
      encoder.clearCount();
      lastEncoderMoveTime = millis();
    } else if (valorEncoder < 0) {
 
      actualizarIndice(false);
      encoder.clearCount();
      lastEncoderMoveTime = millis();
    }

    mostrarMenu();

    if (digitalRead(botonPin) == LOW) {
      delay(50);  // Debounce
      if (digitalRead(botonPin) == LOW) {
        manejarEstado();  // Función que cambia el estado actual
        mostrarMenu();    // Actualiza la pantalla según el nuevo estado
        delay(300);       // Debounce adicional
      }
    }
  }
}

// bool encoderButtonPressed() {
//   static unsigned long lastPressTime = 0;
//   const unsigned long debounceDelay = 30;
//   bool buttonState = digitalRead(botonPin);
//   if (buttonState == LOW) {
//     unsigned long currentTime = millis();
//     if (currentTime - lastPressTime > debounceDelay) {
//       lastPressTime = currentTime;
//       return true;
//     }
//   }
//   return false;
// }
