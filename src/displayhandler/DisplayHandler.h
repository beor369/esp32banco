#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <U8g2lib.h>

// Inicializa la pantalla OLED
void initDisplay();

// Función para dibujar una imagen en la pantalla
void dibujarImagen(const char *nombreImagen);

#endif
