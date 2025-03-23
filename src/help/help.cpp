#include "menu/Menu.h"
#include "displayhandler/DisplayHandler.h"
#include "saveInjector/saveInjector.h"
#include "pruebas/pruebas.h"
#include "pruebas/activatebankofproof/activatebankofproof.h"
#include "encoderhadler/EncoderHandler.h"

#include "InjectorController/InjectorController.h"

int ayudaSelectedOption = 0;
bool ayudaExitHelp = false;
const int ayudaNumPreguntas = 8;
const char* const ayudaPreguntas[ayudaNumPreguntas] = {
  "1. ¿iniciar una prueba?",
  "2. ¿Seleccionar un inyector?",
  "3. ¿modo automático?",
  "4. ¿Nuevo inyector?",
  "5. ¿Ver características?",
  "6. ¿Ver resultados?",
  "7. Atención al Cliente",
  "8. Salir"
};

void mostrarRespuesta(int index) {
  static const char* const respuestas[][3] = {
    { "Para iniciar una prueba: Seleccione 'Iniciar Prueba' en el menu principal. Configure RPM, ancho y tiempo segun necesidad.", NULL, NULL },
    { "Para seleccionar un inyector: Elija el modelo en el submenu 'Seleccionar Inyector'. Si no se encuentra, use 'Agregar Inyector'.", NULL, NULL },
    { "El modo automatico ejecuta pruebas secuenciales en tiempo real.", NULL, NULL },
    { "Para agregar un nuevo inyector: Ingrese a 'Agregar Inyector' en el menu principal e ingrese los datos solicitados.", NULL, NULL },
    { "Para ver las caracteristicas: Seleccione 'Ver Caracteristicas' en el menu correspondiente. Se muestran datos comparativos.", NULL, NULL },
    { "Para ver los resultados obtenidos: Seleccione 'Ver Resultados' en el menu.", NULL, NULL },
    { "Atencion al Cliente: Tel: 3157652018. Correo: bolaya@uts.edu.co.", NULL, NULL }
  };

  const int numRespuestas = sizeof(respuestas) / sizeof(respuestas[0]);
  
  if (index < 0 || index >= numRespuestas) return;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  int x = 0;           // Posición X de inicio del texto
  int y = 10;          // Posición Y inicial
  int maxWidth = 120;  // Ajusta esto según el ancho de tu pantalla
  int lineHeight = 10; // Espacio entre líneas

  // Divide el texto automáticamente en líneas más cortas
  char buffer[200];
  strncpy(buffer, respuestas[index][0], sizeof(buffer));
  buffer[sizeof(buffer) - 1] = '\0';  // Asegurar que esté terminado en NULL

  char* token = strtok(buffer, " ");
  String line = "";

  while (token) {
    String temp = line + token + " ";
    int width = u8g2.getStrWidth(temp.c_str());

    if (width > maxWidth) {
      u8g2.drawStr(x, y, line.c_str());
      y += lineHeight;
      line = String(token) + " ";
    } else {
      line = temp;
    }

    token = strtok(NULL, " ");
  }

  // Imprime la última línea si hay texto pendiente
  if (line.length() > 0) {
    u8g2.drawStr(x, y, line.c_str());
  }


  u8g2.sendBuffer();

  // Espera a que el usuario presione el botón para regresar
  delay(9000);
}

// Callbacks para el encoder
void actualizarIndice_help(bool incremento) {
  ayudaSelectedOption = incremento 
                        ? min(ayudaSelectedOption + 1, ayudaNumPreguntas - 1) 
                        : max(ayudaSelectedOption - 1, 0);
}

void redrawAyudaMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "=== Ayuda - FAQs ===");
  u8g2.drawStr(0, 20, "--------------------");

  int maxLines = 3;  // Número exacto de preguntas visibles
  static int scrollOffset = 0;
  int lineHeight = 12;  // Espaciado adecuado entre líneas
  int startY = 30;  // Posición de inicio del primer ítem

  // Ajustar el desplazamiento cuando la opción seleccionada está fuera del rango visible
  if (ayudaSelectedOption < scrollOffset) {
    scrollOffset = ayudaSelectedOption;
  } else if (ayudaSelectedOption >= scrollOffset + maxLines) {
    scrollOffset = ayudaSelectedOption - maxLines + 1;
  }

  // Dibujar solo las opciones visibles en pantalla
  for (int i = 0; i < maxLines; i++) {
    int idx = scrollOffset + i;
    if (idx >= ayudaNumPreguntas) break;  // No dibujar fuera del rango

    int y = startY + (i * lineHeight);
    if (idx == ayudaSelectedOption) {
      u8g2.drawStr(0, y, ">");
    }
    u8g2.drawStr(10, y, ayudaPreguntas[idx]);
  }

  u8g2.sendBuffer();
}


void manejarEstadoAyuda() {
  if (ayudaSelectedOption == ayudaNumPreguntas - 1) {
    ayudaExitHelp = true;
  } else {
    mostrarRespuesta(ayudaSelectedOption);
  }
}

void mostrarAyuda() {
  ayudaExitHelp = false;
  ayudaSelectedOption = 0;
  redrawAyudaMenu(); // Dibuja el menú inicial

  while (!ayudaExitHelp) {
    displayEncoderPosition(redrawAyudaMenu, actualizarIndice_help, manejarEstadoAyuda);
    delay(10); // Pequeña pausa para evitar sobrecarga
  }
}



