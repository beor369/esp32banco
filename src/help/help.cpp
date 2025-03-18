#include "config.h"
#include <Wire.h>
#include "encoderhadler/EncoderHandler.h"
#include "menu/Menu.h"


// Prototipos de funciones para leer el encoder y detectar botón
int leerEncoder();         // Debe retornar 1 (giro a la derecha), -1 (giro a la izquierda) o 0 (sin cambio)
bool botonSeleccionado();  // Debe retornar true cuando se presiona el botón
void mostrarRespuesta(int index);
// Función para mostrar el menú de ayuda (FAQs)
void mostrarAyuda() {
  bool exitHelp = false;
  int selectedOption = 0;
  const int numPreguntas = 8;
  
  // Array con las preguntas frecuentes y opciones del menú
  const char* preguntas[numPreguntas] = {
    "1. ¿Como iniciar una prueba?",
    "2. ¿Como seleccionar un inyector?",
    "3. ¿Que es el modo automatico?",
    "4. ¿Como agregar un nuevo inyector?",
    "5. ¿Ver caracteristicas?",
    "6. ¿Ver resultados?",
    "7. Atencion al Cliente",
    "8. Salir"
  };

  while (!exitHelp) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "=== Ayuda - FAQs ===");
    u8g2.drawStr(0, 20, "--------------------");
    
    // Muestra las opciones en el menú, resaltando la opción seleccionada
    int y = 30;
    for (int i = 0; i < numPreguntas; i++) {
      if (i == selectedOption) {
        // Dibuja una flecha antes de la opción seleccionada
        u8g2.drawStr(0, y, ">");
        u8g2.drawStr(10, y, preguntas[i]);
      } else {
        u8g2.drawStr(10, y, preguntas[i]);
      }
      y += 10; // Ajusta el espaciado entre líneas según sea necesario
    }
    u8g2.sendBuffer();

    // Lee el movimiento del encoder
    int cambio =encoder.getCount();
    if (cambio > 0 && selectedOption < numPreguntas - 1) {
      selectedOption++;
      delay(200); // Pequeña pausa para evitar múltiples cambios
    } else if (cambio < 0 && selectedOption > 0) {
      selectedOption--;
      delay(200);
    }
    
    // Si se presiona el botón de selección
    if (digitalRead(botonPin)) {
      // Si se selecciona "Salir", se finaliza la función
      if (selectedOption == numPreguntas - 1) {
        exitHelp = true;
      } else {
        // Muestra la respuesta correspondiente a la opción seleccionada
        mostrarRespuesta(selectedOption);
      }
      delay(300);
    }
  }
}

// Función para mostrar la respuesta correspondiente a cada opción del menú
void mostrarRespuesta(int index) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  int y = 10;
  
  switch(index) {
    case 0:
      u8g2.drawStr(0, y, "Para iniciar una prueba:");
      y += 10;
      u8g2.drawStr(0, y, "- Seleccione 'Iniciar Prueba'");
      y += 10;
      u8g2.drawStr(0, y, "  en el menu principal.");
      y += 10;
      u8g2.drawStr(0, y, "- Configure RPM, ancho y");
      y += 10;
      u8g2.drawStr(0, y, "  tiempo segun necesidad.");
      break;
    case 1:
      u8g2.drawStr(0, y, "Para seleccionar un inyector:");
      y += 10;
      u8g2.drawStr(0, y, "- Elija el modelo en el");
      y += 10;
      u8g2.drawStr(0, y, "  submenu 'Seleccionar Inyector'.");
      y += 10;
      u8g2.drawStr(0, y, "- Si no se encuentra, use");
      y += 10;
      u8g2.drawStr(0, y, "  'Agregar Inyector'.");
      break;
    case 2:
      u8g2.drawStr(0, y, "El modo automatico:");
      y += 10;
      u8g2.drawStr(0, y, "- Ejecuta pruebas secuenciales");
      y += 10;
      u8g2.drawStr(0, y, "  en tiempo real.");
      break;
    case 3:
      u8g2.drawStr(0, y, "Para agregar un nuevo inyector:");
      y += 10;
      u8g2.drawStr(0, y, "- Ingrese a 'Agregar Inyector'");
      y += 10;
      u8g2.drawStr(0, y, "  en el menu principal.");
      y += 10;
      u8g2.drawStr(0, y, "- Ingrese los datos solicitados.");
      break;
    case 4:
      u8g2.drawStr(0, y, "Para ver las caracteristicas:");
      y += 10;
      u8g2.drawStr(0, y, "- Seleccione 'Ver Caracteristicas'");
      y += 10;
      u8g2.drawStr(0, y, "  en el menu correspondiente.");
      y += 10;
      u8g2.drawStr(0, y, "- Se muestran datos comparativos");
      break;
    case 5:
      u8g2.drawStr(0, y, "Para ver los resultados obtenidos:");
      y += 10;
      u8g2.drawStr(0, y, "- Seleccione 'Ver Resultados'");
      y += 10;
      u8g2.drawStr(0, y, "  en el menu.");
      break;
    case 6:
      u8g2.drawStr(0, y, "Atencion al Cliente:");
      y += 10;
      u8g2.drawStr(0, y, "Tel: 3157652018");
      y += 10;
      u8g2.drawStr(0, y, "Correo: bolaya@uts.edu.co");
      break;
    default:
      u8g2.drawStr(0, y, "Opcion no disponible.");
      break;
  }
  
  y += 20;
  u8g2.drawStr(0, y, "Presione el boton");
  y += 10;
  u8g2.drawStr(0, y, "para volver...");
  u8g2.sendBuffer();
  
  // Espera a que el usuario presione el botón para regresar al menú de ayuda
  while (!botonSeleccionado()) {
    delay(100);
  }
}

// Funciones ficticias para simular la lectura del encoder y del botón
// Estas funciones deben ser implementadas de acuerdo a tu hardware

int leerEncoder() {
  // Retorna 1, -1 o 0 según el giro detectado.
  // Implementa aquí la lectura real de tu encoder.
  return 0;
}

bool botonSeleccionado() {
  // Retorna true si el botón ha sido presionado.
  // Implementa aquí la lectura real de tu botón.
  return false;
}

// void setup() {
//   u8g2.begin();
//   Serial.begin(115200);
//   // Configura los pines del encoder y del botón según tu hardware.
// }

// void loop() {
//   // Invoca la función de ayuda cuando se requiera
//   mostrarAyuda();
//   // Resto del programa...
//   delay(1000);
// }
