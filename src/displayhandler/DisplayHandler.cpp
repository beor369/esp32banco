#include "DisplayHandler.h"
#include "images/imagenes.h"  // Asumiendo que aquí tienes las imágenes definidas
#include "Config.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

void initDisplay() {
  u8g2.begin();
}

void dibujarImagen(const char *nombreImagen) {
  Serial.println(String("imagen recibida: ") + nombreImagen);

  u8g2.clearBuffer();
  // Se asume que Imagenes::obtenerImagen() devuelve el array adecuado
  u8g2.drawXBMP(0, 0, 128, 64, Imagenes::obtenerImagen(nombreImagen));
  u8g2.sendBuffer();
}
