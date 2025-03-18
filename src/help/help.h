#ifndef HELP_H
#define HELP_H

#include "config.h"
#include "encoderhadler/EncoderHandler.h"
#include "menu/Menu.h"
// Declaración de funciones del módulo de ayuda
void mostrarAyuda();
void mostrarRespuesta();
//void mostrarAyuda();

// Declaración de funciones para la lectura del encoder y del botón
// Estas funciones deben implementarse de acuerdo a tu hardware.
int leerEncoder();
bool botonSeleccionado();

#endif // HELP_H
