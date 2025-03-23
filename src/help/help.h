#ifndef HELP_H
#define HELP_H

#include <U8g2lib.h> // Asegúrate de incluir la librería necesaria

// Variables globales para el menú de ayuda
extern int ayudaSelectedOption;
extern bool ayudaExitHelp;


// Funciones del menú de ayuda
void actualizarIndice_help(bool incremento);
void redrawAyudaMenu();
void manejarEstadoAyuda();
extern void mostrarAyuda();
void mostrarRespuesta(int opcion); 

#endif // HELP_H
