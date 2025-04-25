#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <ESP32Encoder.h>
#include <EEPROM.h>
#include "pruebas/activatebankofproof/activatebankofproof.h"
#include "InjectorController/InjectorController.h"
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <ToneESP32.h>
#include "pruebas/pruebas.h"
#include "Notes.h"

// Lee la temperatura en grados Celsius


// Pines del encoder
#define ENCODER_PIN_A 41
#define ENCODER_PIN_B 42

// Pin del botón del encoder
const int botonPin = 40;
#define DEBOUNCE_DELAY 10  // Tiempo de debounce en milisegundos

// Pin del buzzer
#define BUZZER_PIN 19
#define BUZZER_CHANNEL 9
#define BUZZER_CHAN     1   // distinto al 0 que usa tu otro módulo
#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988


#define BOMBA_PIN 17
// extern bool bombaActiva; // Variable para rastrear el estado
//pin relay inyector
#define PIN_RELAY_INYECTOR_ALIM 13
#define INYECTOR_PIN 12
//extern int pinled = 2;
// Configuración del PWM
#define PIN_INYECTOR_PWM 5  // Pin conectado al MOSFET (ej: GPIO9)
#define PWM_CHANNEL 0       // Canal PWM (0-15)
#define PWM_FREQ 100        // Frecuencia en Hz (ajusta según el inyector)
#define PWM_RESOLUTION 8    // Resolución de 8 bits (0-255)
#define pinraro 18
// Variables de control
extern bool inyectorActivo;
// Pines de los sensores
//PIN ACS712
#define PIN_CORRIENTE 30
//PIN F-S201
#define PIN_FLUJO 39
//Termistor NTC
#define PIN_TEMP 6
// SENSOR DE SONIDO
#define PIN_SONIDO 7       // GPIO7 (ADC1_CH6)
#define UMBRAL_CLIC 10000   // Ajustar según el sensor (valor empírico)
#define TIEMPO_ESPERA 50 // Máximo tiempo para detectar clic (ms)

// Variables
extern bool clicDetectado;



// Tamaño de la EEPROM
const int EEPROM_SIZE = 512;

// Enumeración de estados
enum Estado {
  MENU_SELECCION_INYECTOR,
  MENU_PRINCIPAL,
  MENU_RESISTENCIA,
  MENU_PRUEBA,
  SUBMENU_MANUAL,
  SUBMENU_AUTOMATICO,
  SUBMENU_MID_RES,
  SUBMENU_FLUJO,
  SUBMENU_FUGAS,
  SUBMENU_CLICK,
  SUBMENU_ABANICO,
  SUBMENU_TEMPERATURA,
  SUBMENU_TIEMPO_RESPUESTA,
  SUBMENU_CORRIENTE_ACTIVACION,
  SUBMENU_RESULTADOS,
  SUBMENU_FUNCIONAMIENTO,
  AYUDA,
  ATRAS,
  DEVOLVER,
  AGREGAR_INJECTOR,
  MENU_CARACTERISTICAS,
  ATRASITO,
  SELECCIONAR_MOTO,
  SELECCIONAR_RPM_TIME,
  SIGUIENTE,
  ATRASSE,
  MENU_CONFIRMA_MOTO,
  AGREGAR_BORRAR_INJ,
  AGREGAR,
  BORRAR,
  SUBMENU_CORRECCIONES,
  TECLADO_VIRTUAL
};

extern Estado estadoActual;

// Variables globales para manejar los índices en el menú
extern int indiceMenu;
extern int indiceInyector;
extern int indiceSubMenu;
extern int indiceSubSubMenu;
extern int indicefuncionamiento;
extern int indiceseleccionarmoto;
extern int indiceMotoSeleccionada;
extern int indiceagregarborrarinjt;

// Instancias globales que pueden usarse en varios módulos
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern ESP32Encoder encoder;

extern TwoWire ina219Wire;
extern Adafruit_INA219 ina219;

// Parámetros de tiempo para el encoder
const unsigned long encoderMoveDelay = 100;
extern unsigned long lastEncoderMoveTime;

#endif
