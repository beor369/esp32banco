#include <Arduino.h>
#include "Config.h"
#include "displayhandler/DisplayHandler.h"
#include "encoderhadler/EncoderHandler.h"
#include "menu/Menu.h"
#include "tecladovirtual/TecladoVirtual.h"

// Instanciar objetos globalmente
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 20, 21);
ESP32Encoder encoder;

// Variables globales de estado (puedes inicializarlas aquí)
Estado estadoActual = MENU_PRINCIPAL;
int indiceMenu = 0;
int indiceInyector = 0;
int indiceSubMenu = 0;
int indiceSubSubMenu = 0;
int indicefuncionamiento = 0;
int indiceseleccionarmoto = 0;
int indiceMotoSeleccionada = 0;
int indiceagregarborrarinjt = 0;
unsigned long lastEncoderMoveTime = 0;

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, UART_CAM_RX, UART_CAM_TX);
  EEPROM.begin(EEPROM_SIZE);

  initDisplay();
  setupEncoder();
  mostrarMenu(); // Muestra el menú inicial
}

void loop()
{
  displayEncoderPosition();
  // Otras tareas que necesites ejecutar en el loop
}
