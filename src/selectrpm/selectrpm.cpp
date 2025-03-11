#include <Arduino.h>
#include "Menu/Menu.h"
#include "config.h"
#include <U8g2lib.h>
#include "selectrpm/selectrpm.h"
#include <Arduino.h>
#include "InjectorController/InjectorController.h"
// #include <ESP32Encoder.h>
//  #include "encoderhadler/EncoderHandler.h"
//  ============================
//  Inicialización del Encoder
//  ============================
//  ESP32Encoder encoder;




State currentState = SELECT_RPM;

long rpmValue = 1200;       // Base 1000 RPM
long pulseWidthValue = 2;  // Base 10 ms
long  testTimeValue = 50000; // Base 10,000 ms (10 s)

// Variables para debounce del botón
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200; // 200 ms

// ============================
// Función para Mostrar el Menú en la Pantalla OLED
// ============================
void displayMenu()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  switch (currentState)
  {
  case SELECT_RPM:
  {
    u8g2.drawStr(0, 10, "Seleccionar RPM:");
    char buf[20];
    sprintf(buf, "RPM: %ld", rpmValue);
    // Serial.print(rpmValue);
    u8g2.drawStr(0, 25, buf);
    u8g2.drawStr(0, 45, "Gira encoder");
    u8g2.drawStr(0, 60, "Presiona para OK");
    break;
  }
  case SELECT_PULSE:
  {
    u8g2.drawStr(0, 10, "Seleccionar pulso:");
    char buf[20];
    sprintf(buf, "Pulso: %ld ms", pulseWidthValue);
    u8g2.drawStr(0, 25, buf);

    //  Serial.print(pulseWidthValue);
    u8g2.drawStr(0, 45, "Gira encoder");
    u8g2.drawStr(0, 60, "Presiona para OK");
    break;
  }
  case SELECT_TIME:
  {
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
void activarInyector(unsigned long tiempoOn, unsigned long tiempoOff = 0)
{
 

digitalWrite(INYECTOR_PIN, HIGH);
delay(tiempoOn);
digitalWrite(INYECTOR_PIN, LOW);
  if (tiempoOff > 0)
  {
    delay(tiempoOff);
  }
}

// ============================
void simularRPM(unsigned int rpm, unsigned long testDuration, unsigned long pulseTime) {
  // Calcula el período (ms) para cada pulso según las RPM
     // Cuenta regresiva mostrada en la pantalla
     unsigned long tiempoSeg = testTimeValue / 1000; // Conversión a segundos

  // for (unsigned long i = tiempoSeg; i > 0; i--) {
    // Muestra la configuración inicial de la prueba
      // Ejecuta la prueba según los parámetros seleccionados

u8g2.clearBuffer();

u8g2.setFont(u8g2_font_ncenB08_tr);
u8g2.drawStr(0, 10, "Iniciando prueba");
char buf[32];
sprintf(buf, "RPM: %ld", rpmValue);
u8g2.drawStr(0, 25, buf);
sprintf(buf, "Pulso: %ld ms", pulseWidthValue);
u8g2.drawStr(0, 40, buf);
  // sprintf(buf, "Tiempo: %lus", i);
  sprintf(buf, "Tiempo: %ld s", testTimeValue / 1000);
  u8g2.drawStr(0, 55, buf);
  u8g2.sendBuffer();
  // delay(1000);
  digitalWrite(BOMBA_PIN, LOW);
delay(2000);
 
// }
  unsigned long periodo = 60000UL / rpm;
  if (pulseTime >= periodo) {
    pulseTime = periodo / 2;
  }
  
  unsigned long startTime = millis();
  unsigned long endTime = startTime + testDuration;
  
  while (millis() < endTime) {
    // Activa el inyector durante el ancho de pulso definido
    activarInyector(pulseTime);
    
    // Espera el resto del período para mantener la frecuencia según RPM
    unsigned long espera = periodo - pulseTime;
    if (espera > 0) {
      delay(espera);
    }
  }
  digitalWrite(BOMBA_PIN, HIGH);
}

// ============================
// setup() y loop()
// ============================
void setupselectrpm()
{
  pinMode(INYECTOR_PIN, OUTPUT);
  pinMode(BOMBA_PIN, OUTPUT);
 digitalWrite(INYECTOR_PIN,LOW);
 digitalWrite(BOMBA_PIN,HIGH);

  // displayMenu();
}

void actualizarIndic(bool incremento)
{
  if (incremento)
  {
    if (currentState == SELECT_RPM)
    {
      rpmValue += 500;
      if (rpmValue > 12000)
        rpmValue = 12000; // Límite superior
    }

    if (currentState == SELECT_PULSE)
    {
      pulseWidthValue += 1;
      if (pulseWidthValue > 20)
        pulseWidthValue = 20; // Límite superior
    }

    if (currentState == SELECT_TIME)
    {
      testTimeValue += 50000;
      if (testTimeValue > 300000)
        testTimeValue = 300000; // Límite de 5 minutos
    }
  }
  else // Resta de valores
  {
    if (currentState == SELECT_RPM)
    {
      rpmValue -= 500;
      if (rpmValue < 1200)
        rpmValue = 1200; // Límite inferior
    }

    if (currentState == SELECT_PULSE)
    {
      pulseWidthValue -= 1;
      if (pulseWidthValue < 2)
        pulseWidthValue = 2; // Límite inferior
    }

    if (currentState == SELECT_TIME)
    {
      testTimeValue -= 50000;
      if (testTimeValue < 10000)
        testTimeValue = 10000; // Límite inferior
    }
  }

}

void displayEncoderPositionRPM()
{
  int valorEncoder = encoder.getCount();

  if (valorEncoder > 0)
  {
    actualizarIndic(true); // Función definida en Menu.cpp
    encoder.clearCount();
  }
  else if (valorEncoder < 0)
  {
    actualizarIndic(false);
    encoder.clearCount();
  }
}

// Función para verificar la pulsación del botón del encoder y cambiar de estado
void checkEncoderButton()
{
  delay(200);
  // Verificar pulsación del botón (nivel LOW) con debounce
  if (millis() - lastButtonPress > debounceDelay)
  {
    if (digitalRead(botonPin) == LOW)
    { // 'botonPin' debe estar definido globalmente
      lastButtonPress = millis();
      // Cambia el estado según el parámetro actual

      switch (currentState)
      {
      case SELECT_RPM:
        currentState = SELECT_PULSE;
        encoder.clearCount();
        break;
      case SELECT_PULSE:
        currentState = SELECT_TIME;
        encoder.clearCount();
        break;
      case SELECT_TIME:
        estadoActual=SELECCIONAR_MOTO;
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
void updateEncoderParameters()
{
  switch (currentState)
  {
  case SELECT_RPM:
    displayEncoderPositionRPM();
    break;
  case SELECT_PULSE:
    displayEncoderPositionRPM();
    break;
  case SELECT_TIME:
    displayEncoderPositionRPM();
    break;
  default:
    break;
  }
}

// Función principal que integra la selección y ejecución de la prueba
void loopselectrpm()
{
  // Llama a la función que verifica el botón del encoder
  checkEncoderButton();

  // Actualiza el valor de los parámetros según la posición del encoder
  updateEncoderParameters();

  // Actualiza el menú en la pantalla
  displayMenu();

  encoder.clearCount();
  delay(100); // Pequeña pausa para no saturar la CPU
}
