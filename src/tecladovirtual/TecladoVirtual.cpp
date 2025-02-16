#include "TecladoVirtual.h"
#include "Config.h"
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern ESP32Encoder encoder;

String tecladoVirtual() {
  const char teclado[3][4][11] = {
    { // Modo 0: Mayúsculas
      {'A','B','C','D','E','F','G','H','I','J','K'},
      {'L','M','N','O','P','Q','R','S','T','U','V'},
      {'W','X','Y','Z','1','2','3','4','5','6','7'},
      {'8','9','0','.','-','<','>','*',' ',' ',' '}
    },
    { // Modo 1: Minúsculas
      {'a','b','c','d','e','f','g','h','i','j','k'},
      {'l','m','n','o','p','q','r','s','t','u','v'},
      {'w','x','y','z','1','2','3','4','5','6','7'},
      {'8','9','0','.','-','<','>','*',' ',' ',' '}
    },
    { // Modo 2: Caracteres especiales
      {'!','@','#','$','%','^','&','*','(',')','_'},
      {'+','=','<','>','?','/','[',']','{','}','|'},
      {'~','`',':',';','"','\'','\\',' ',' ',' ',' '},
      {' ',' ',' ',' ',' ','<','>','*',' ',' ',' '}
    }
  };

  String textoSeleccionado = "";
  int fila = 0, columna = 0;
  int lastEncoderPosition = encoder.getCount();
  bool seleccionando = true;
  int modo = 0; // 0 = Mayúsculas, 1 = Minúsculas, 2 = Especiales

  while (seleccionando) {
    int encoderPosition = encoder.getCount();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);

    if (encoderPosition != lastEncoderPosition) {
      if (encoderPosition > lastEncoderPosition) {
        columna++;
        if (columna > 10) {
          columna = 0;
          fila = (fila + 1) % 4;
        }
      } else {
        columna--;
        if (columna < 0) {
          columna = 10;
          fila = (fila - 1 + 4) % 4;
        }
      }
      lastEncoderPosition = encoderPosition;
    }

    // Seleccionar carácter si se presiona el botón
    if (digitalRead(botonPin) == LOW) {
      char seleccion = teclado[modo][fila][columna];
      if (seleccion == '-') {
        if (textoSeleccionado.length() > 0)
          textoSeleccionado.remove(textoSeleccionado.length() - 1);
      } else if (seleccion == '>') {
        seleccionando = false;
      } else if (seleccion == '<') {
        return "";
      } else if (seleccion == '*') {
        modo = (modo + 1) % 3;
      } else {
        textoSeleccionado += seleccion;
      }
      delay(200);
    }

    // Dibujar el teclado y el texto seleccionado
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 11; j++) {
        int x = j * 11;
        int y = (i + 1) * 12;
        if (i == fila && j == columna) {
          u8g2.drawStr(x, y, "[");
          u8g2.drawStr(x + 9, y, "]");
          u8g2.drawStr(x + 3, y, String(teclado[modo][i][j]).c_str());
        } else {
          u8g2.drawStr(x, y, String(teclado[modo][i][j]).c_str());
        }
      }
    }
    u8g2.drawStr(0, 60, textoSeleccionado.c_str());
    u8g2.sendBuffer();
  }
  return textoSeleccionado;
}
