#include "pruebas/activatebankofproof/activatebankofproof.h"
#include "Arduino.h"
#include "Config.h"
#include <Arduino.h>
#include <ESP32_PWM.h>
//#include <AnalogWrite.h>  // Incluir la librería
#include "InjectorController/injectorController.h"
#include "selectrpm/selectrpm.h"

// void activarInyector() {
//   // 1. Configurar frecuencia (requiere modificar la librería LEDC subyacente)
//   analogWriteFrequency(PIN_INYECTOR, frecuencia); // Configurar frecuencia

//   // 2. Calcular el duty cycle (0-255 para 8 bits)
//   uint8_t duty = (anchoPulso * frecuencia) / 1000000.0 * 255;

//   // 3. Activar PWM
//   analogWrite(PIN_INYECTOR, duty);
//   Serial.println("Inyector ACTIVADO");

//   // 4. Mantener activo por tiempo de prueba
//   delay(tiempoPrueba * 1000);

//   // 5. Desactivar
//   analogWrite(PIN_INYECTOR, 0);
//   Serial.println("Inyector DESACTIVADO");
// }


//   analogWriteResolution(PIN_INYECTOR, 8); // Resolución de 8 bits (0-255)


// void loop() {
//   if (Serial.available()) {
//     String input = Serial.readStringUntil('\n');
    // (Aquí iría el parsing de parámetros similar al ejemplo anterior)
  //  activarInyector();
  
// Función para activar el buzzer
// void beep() {
//   if (!buzzerActive) {
//     digitalWrite(BUZZER_PIN, HIGH); // Activa el buzzer
//     buzzerStartTime = millis();     // Guarda el tiempo de inicio
//     buzzerActive = true;            // Marca como activo
//   }
// }

// // Función para actualizar el estado del buzzer (llamar en loop())
// void updateBuzzer() {
//   if (buzzerActive && (millis() - buzzerStartTime >= 1000)) {
//     digitalWrite(BUZZER_PIN, LOW);  // Apaga el buzzer después de 1 segundo
//     buzzerActive = false;           // Marca como inactivo
//   }
// }

// void setupbit() {
//   pinMode(BUZZER_PIN, OUTPUT);      // Configura el pin como salida
//   digitalWrite(BUZZER_PIN, LOW);    // Asegura que el buzzer inicia apagado
// }




// Función: controlBomba(activar)
// - activar: true (encender), false (apagar)

// // Inicializar PWM
// void setupPWM() {
//     ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
//     ledcAttachPin(PIN_INYECTOR_PWM, PWM_CHANNEL);
//     ledcWrite(PWM_CHANNEL, 0); // Inicia apagado
//   }
  
  // Función: activarInyector(duracion_ms, duty_cycle)
  // - duracion_ms: Tiempo de activación en milisegundos.
  // - duty_cycle: Porcentaje de apertura (0-100%).

//prueba de sonido
  void setupmic() {

    pinMode(PIN_SONIDO, INPUT);
    pinMode(PIN_SONIDO, LOW);
  //  analogWriteResolution(INYECTOR_PIN, 8); // Resolución de 8 bits (0-255)

  }
  // void activarInyectorCorto(unsigned long pulseWidth) {
  //   // Activa el inyector por un pulso corto (ej: 5 ms)
  //   digitalWrite(INYECTOR_PIN, HIGH);
  //   delay(pulseWidth);
  //   digitalWrite(INYECTOR_PIN, LOW);
  // }

  // Función para detectar el clic
// bool detectarClic() {
//   int dutyCycle = 220;  // Ajusta este valor para enviar más o menos energía
  
//   Serial.print("Activando inyector con dutyCycle: ");
//   Serial.println(dutyCycle);
  
//   // Envía el pulso al inyector
//   ledcWrite(ledChannel, dutyCycle);
//   delay(100);  // Duración del pulso en milisegundos (puedes aumentarlo si es necesario)
//   ledcWrite(ledChannel, 0);
  
//   // Serial.println("DETECTAR CLIC");
//   //   clicDetectado = false;
//      unsigned long inicio = millis();
//   //   digitalWrite(INYECTOR_PIN, HIGH);
//   //   delay(1000);
//   //   digitalWrite(INYECTOR_PIN, LOW);

//     // Muestrear durante TIEMPO_ESPERA ms

//     while (millis() - inicio < TIEMPO_ESPERA) {
//       for (brillo = 0; brillo <=255; brillo++){

//        analogWrite(ledChannel, brillo);
//         delay(15);
//       }
//       for (brillo = 0; brillo <=255; brillo--){

//         analogWrite(ledChannel, brillo);
//         delay(15);
//       }
//       int lectura = analogRead(PIN_SONIDO);
      
//       // Si supera el umbral, hay clic
//       if (lectura > UMBRAL_CLIC) {
//         clicDetectado = true;
//         break;
//       }
//     }

//     return clicDetectado;
    
//   }
  // void activateInjector(int rpm, int pulseWidthMs, unsigned long testDuration) {
  //   // Calcular la frecuencia para 1 pulso por revolución: rpm/60 Hz.
  //   float frequency = (float)rpm / 60.0;
    
  //   // Calcular el período del PWM en milisegundos.
  //   float periodMs = 1000.0 / frequency;
    
  //   // Validar que el ancho de pulso no exceda el período
  //   if (pulseWidthMs > periodMs) {
  //     Serial.print("Error: El ancho de pulso (");
  //     Serial.print(pulseWidthMs);
  //     Serial.print(" ms) excede el período del PWM (");
  //     Serial.print(periodMs);
  //     Serial.println(" ms).");
  //     return;
  //   }
    
  //   // Calcular el duty cycle (en %) correspondiente
  //   float dutyCyclePercentage = (pulseWidthMs / periodMs) * 100.0;
    
  //   Serial.print("RPM: ");
  //   Serial.print(rpm);
  //   Serial.print(", Frecuencia: ");
  //   Serial.print(frequency);
  //   Serial.print(" Hz, Periodo: ");
  //   Serial.print(periodMs);
  //   Serial.print(" ms, Duty Cycle: ");
  //   Serial.print(dutyCyclePercentage);
  //   Serial.println(" %");
    
  //   // Configurar el PWM en el pin con la frecuencia y el duty cycle calculado.
  //   pwm.setPWM(PIN_INYECTOR_PWM, frequency, dutyCyclePercentage);
    
  //   // Mantener la señal activa durante el tiempo de prueba
  //   delay(testDuration);
    
  //   // Apagar el inyector poniendo el duty cycle a 0
  //   pwm.setPWM(PIN_INYECTOR_PWM, frequency, 0);
    
  //   Serial.println("Prueba de activación del inyector completada.");
  // }
  // Prueba completa del clic
  void pruebaClic() {
    // Activar inyector (pulso corto)
   // activarInyectorCorto(5);
    // Detectar clic
    
    Serial.println("PRUEBA CLIC");
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.clearBuffer();
    u8g2.drawStr(0, 30, "INICIANDO PRUEBA  ");
    u8g2.sendBuffer();
    delay(2000); 
  } 
    // Pequeño retardo para estabilidad
    // Serial.println("Inicio de prueba: 1 pulso = 1 revolución");
    //activarInyector();
    // Ejemplo: para 5000 rpm con un pulso de 1 ms por revolución durante 10 segundos.
    // activateInjector(5000, 1, 10000);
  //   while (!Serial);
  //   delay(2000);
  // En cualquier función o clase:
//inyector.activate(50.0, 2000, 10); // Frecuencia, ancho de pulso, duración

// Detener manualmente:

//inyector.stop();

// Verificar estado:
// if (inyector.isInjectorActive()) {
//   // Hacer algo mientras está activo
// }
  //}
  //   Serial.println("Inicio de configuración del PWM para el inyector.");
    
  //   // Configurar PWM para 5000 rpm con 1 ms (1000 us) de pulso inicial
  //   uint32_t pwmChannel = setupInjectorPWM(5000.0, 1000);
    
  //   // Después de 5 segundos, aumentar el ancho de pulso a 2000 us para ver el efecto
  //   delay(5000);
  //   if (pwmChannel != 0xFFFF)
  //   {
  //     updateInjectorPulseWidth(pwmChannel, 5000.0, 2000);
  //   }
  // }
    //  bool resultado = detectarClic();
    //  u8g2.clearBuffer();
    // // Mostrar resultado
    // if (resultado) {
    //   u8g2.setFont(u8g2_font_ncenB08_tr);
    //   u8g2.clearBuffer();
    //   u8g2.drawStr(0, 10, "click detectado ");
    //   u8g2.sendBuffer();
    //   Serial.println("✔️ Clic detectado - Inyector funcional");
    //   delay (5000);
    //   estadoActual = SUBMENU_MANUAL;
    // } else {
    //   u8g2.setFont(u8g2_font_ncenB08_tr);
    //   u8g2.clearBuffer();
    //   u8g2.drawStr(0, 10, "click NO detectado ");
    //   u8g2.sendBuffer();
    //   Serial.println("❌ Fallo: No se detectó clic");
    //   delay (5000);
    //   estadoActual = SUBMENU_MANUAL;
    // }

  
   // Test de Sonido
    // char output[50]; // Buffer para almacenar el texto a mostrar
    // result = runTestSonido(selected);
    // sprintf(output, "Sonido: %.2f %s", result.measuredValue, result.passed ? "OK" : "FALLA");
    // u8g2.clearBuffer();
    // u8g2.drawStr(0, 12, output);
    // u8g2.sendBuffer();
    // delay(1000);
 