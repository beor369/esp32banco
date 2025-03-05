#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <ESP32Encoder.h>
#include <EEPROM.h>
#include "pruebas/activatebankofproof/activatebankofproof.h"

// Pines del encoder
#define ENCODER_PIN_A 41
#define ENCODER_PIN_B 42

// Pin del botón del encoder
const int botonPin = 40;
#define DEBOUNCE_DELAY 10  // Tiempo de debounce en milisegundos

// Pin del buzzer
#define BUZZER_PIN 19
// Pine de relay bomba
#define PIN_BOMBA 17
extern bool bombaActiva; // Variable para rastrear el estado
//pin relay inyector
#define PIN_RELAY_INYECTOR_ALIM 13
#define INYECTOR_PIN 12
// Configuración del PWM
#define PIN_INYECTOR_PWM 9  // Pin conectado al MOSFET (ej: GPIO9)
#define PWM_CHANNEL 0       // Canal PWM (0-15)
#define PWM_FREQ 100        // Frecuencia en Hz (ajusta según el inyector)
#define PWM_RESOLUTION 8    // Resolución de 8 bits (0-255)

// Variables de control
extern bool inyectorActivo;
// Pines de los sensores
//PIN ACS712
#define PIN_CORRIENTE 4
//PIN F-S201
#define PIN_FLUJO 5
//Termistor NTC
#define PIN_TEMP 6
// SENSOR DE SONIDO
#define PIN_SONIDO 7       // GPIO7 (ADC1_CH6)
#define UMBRAL_CLIC 2000   // Ajustar según el sensor (valor empírico)
#define TIEMPO_ESPERA 50   // Máximo tiempo para detectar clic (ms)

// Variables
extern bool clicDetectado;

// Configuración PWM (usando ledc)
extern const int pwmChannell;
extern const int pwmFreqq;    // Hz
extern const int pwmResolutionn;
#define LED_STATUS 2


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
  SIGUIENTE,
  ATRASSE,
  MENU_CONFIRMA_MOTO,
  AGREGAR_BORRAR_INJ,
  AGREGAR,
  BORRAR,
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

// Parámetros de tiempo para el encoder
const unsigned long encoderMoveDelay = 200;
extern unsigned long lastEncoderMoveTime;

#endif
