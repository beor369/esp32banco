#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <ESP32Encoder.h>
#include <EEPROM.h>

// Pines del encoder
#define ENCODER_PIN_A 42
#define ENCODER_PIN_B 41

// Pines de comunicación UART con ESP32-CAM
#define UART_CAM_RX 16
#define UART_CAM_TX 17

// Pin del botón del encoder
const int botonPin = 40;
#define DEBOUNCE_DELAY 10  // Tiempo de debounce en milisegundos

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
