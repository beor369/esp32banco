#include <Arduino.h>
#include "Menu/Menu.h"
#include "config.h"
#include <U8g2lib.h>
//#include <ESP32Encoder.h>
// #include "encoderhadler/EncoderHandler.h"
// ============================
// Inicialización del Encoder
// ============================
// ESP32Encoder encoder;

#define MAX_RPM             10000 // Límite máximo de RPM
#define MAX_PULSE_WIDTH     100   // Límite máximo de ancho de pulso (ms)
#define MAX_TEST_TIME       60000 // Límite máximo de tiempo de prueba (ms)

// ============================
// Estados del Menú y Variables de Parámetros
// ============================
enum State { SELECT_RPM, SELECT_PULSE, SELECT_TIME, RUN_TEST };
State currentState = SELECT_RPM;

// Valores base iniciales
extern long rpmValue = 1000;         // Base 1000 RPM
extern long pulseWidthValue = 10;    // Base 10 ms
extern long testTimeValue = 10000;   // Base 10,000 ms (10 s)

// Variables para debounce del botón
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200; // 200 ms

// ============================
// Función para Mostrar el Menú en la Pantalla OLED
// ============================
void displayMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  switch (currentState) {
    case SELECT_RPM: {
      u8g2.drawStr(0, 10, "Seleccionar RPM:");
      char buf[20];
      sprintf(buf, "RPM: %ld", rpmValue);
      //Serial.print(rpmValue);
      u8g2.drawStr(0, 25, buf);
      u8g2.drawStr(0, 45, "Gira encoder");
      u8g2.drawStr(0, 60, "Presiona para OK");
      break;
    }
    case SELECT_PULSE: {
      u8g2.drawStr(0, 10, "Seleccionar pulso:");
      char buf[20];
      sprintf(buf, "Pulso: %ld ms", pulseWidthValue);
      u8g2.drawStr(0, 25, buf);
    
    //  Serial.print(pulseWidthValue);
      u8g2.drawStr(0, 45, "Gira encoder");
      u8g2.drawStr(0, 60, "Presiona para OK");
      break;
    }
    case SELECT_TIME: {
      u8g2.drawStr(0, 10, "Seleccionar tiempo:");
      char buf[20];
      sprintf(buf, "Tiempo: %ld s", testTimeValue / 1000);
      u8g2.drawStr(0, 25, buf);
      u8g2.drawStr(0, 45, "Gira encoder");
      u8g2.drawStr(0, 60, "Presiona para OK");
      break;
    }
    case RUN_TEST:
      u8g2.drawStr(0, 30, "Ejecutando prueba...");
      break;
  }
  u8g2.sendBuffer();
}

// ============================
// Función para Activar el Inyector
// ============================
void activarInyector(unsigned long tiempoOn, unsigned long tiempoOff = 0) {
  digitalWrite(INYECTOR_PIN, HIGH);
  delay(tiempoOn);
  digitalWrite(INYECTOR_PIN, LOW);
  if (tiempoOff > 0) {
    delay(tiempoOff);
  }
}

// ============================
// Función para Simular la Prueba de Llenado
// ============================
void simularRPM(unsigned int rpm, unsigned long testDuration, unsigned long pulseTime) {
  // Calcula el período (ms) para cada pulso según las RPM
  unsigned long periodo = 60000UL / rpm;
  // Asegura que el ancho de pulso no sea mayor que el período
  if (pulseTime >= periodo) {
    pulseTime = periodo / 2;
  }
  
  unsigned long startTime = millis();
  while (millis() - startTime < testDuration) {
    activarInyector(pulseTime);
    unsigned long espera = periodo - pulseTime;
    if (espera > 0) {
      delay(espera);
    }
  }
}

// ============================
// setup() y loop()
// ============================
void setupselectrpm() {
  pinMode(INYECTOR_PIN, OUTPUT);

  displayMenu();
}

// Función para verificar la pulsación del botón del encoder y cambiar de estado
void checkEncoderButton() {
    // Verificar pulsación del botón (nivel LOW) con debounce
    if (millis() - lastButtonPress > debounceDelay) {
      if (digitalRead(botonPin) == LOW) {  // 'botonPin' debe estar definido globalmente
        lastButtonPress = millis();
        // Cambia el estado según el parámetro actual
        switch (currentState) {
          case SELECT_RPM:
            currentState = SELECT_PULSE;
            encoder.clearCount();
            break;
          case SELECT_PULSE:
            currentState = SELECT_TIME;
            encoder.clearCount();
            break;
          case SELECT_TIME:
            currentState = RUN_TEST;
            break;
          default:
            break;
        }
        displayMenu();
        delay(100); // Pequeña pausa para evitar rebotes
      }
    }
  }
  
  // Función para actualizar los parámetros en función de la posición del encoder
  void updateEncoderParameters() {
    int count = encoder.getCount();
    Serial.print("Count: ");
    Serial.println(count);
    
    switch (currentState) {
      case SELECT_RPM:
        rpmValue = 1000 + count * 100;  // Cada "paso" suma 100 RPM
        Serial.print("RPM: ");
        Serial.println(rpmValue);
        if (rpmValue < 1000) rpmValue = 1000;
        if (rpmValue > MAX_RPM) rpmValue = MAX_RPM;
        break;
      case SELECT_PULSE:
        pulseWidthValue = 10 + count;   // Cada paso suma 1 ms
        if (pulseWidthValue < 10) pulseWidthValue = 10;
        if (pulseWidthValue > MAX_PULSE_WIDTH) pulseWidthValue = MAX_PULSE_WIDTH;
        break;
      case SELECT_TIME:
        testTimeValue = 10000 + count * 1000;  // Cada paso suma 1 s (1000 ms)
        if (testTimeValue < 10000) testTimeValue = 10000;
        if (testTimeValue > MAX_TEST_TIME) testTimeValue = MAX_TEST_TIME;
        break;
      default:
        break;
    }
  }
  
  // Función principal que integra la selección y ejecución de la prueba
  void loopselectrpm() {
    // Llama a la función que verifica el botón del encoder
    checkEncoderButton();
    
    // Actualiza el valor de los parámetros según la posición del encoder
    updateEncoderParameters();
    
    // Actualiza el menú en la pantalla
    displayMenu();
    
    // Cuando se confirma la selección (estado RUN_TEST), se ejecuta la prueba
    if (currentState == RUN_TEST) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(0, 10, "Iniciando prueba de llenado");
      char buf[32];
      sprintf(buf, "RPM: %ld", rpmValue);
      u8g2.drawStr(0, 25, buf);
      sprintf(buf, "Pulso: %ld ms", pulseWidthValue);
      u8g2.drawStr(0, 40, buf);
      sprintf(buf, "Tiempo: %ld s", testTimeValue / 1000);
      u8g2.drawStr(0, 55, buf);
      u8g2.sendBuffer();
      delay(1000);
      
      // Ejecuta la prueba según los parámetros seleccionados
      simularRPM(rpmValue, testTimeValue, pulseWidthValue);
      
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(0, 30, "Prueba finalizada");
      u8g2.sendBuffer();
      delay(2000);
      
      // Reinicia el menú para una nueva selección
      currentState = SELECT_RPM;
      encoder.clearCount();
      displayMenu();
    }
    
    delay(100); // Pequeña pausa para no saturar la CPU
  }
  