#ifndef ACTIVATE_BANK_OF_PROOF_H
#define ACTIVATE_BANK_OF_PROOF_H
//generar un sonido
void setupbit();
void beep();
void updateBuzzer();
//control de bomba y injector
void controlBomba(bool activar);
void setupbomba();
void setupPWM();
//prueba de sonido
void setupmic();
void pruebaClic();
bool detectarClic();
void setupi();
void loopi();
#endif 