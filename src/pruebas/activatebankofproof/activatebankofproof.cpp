#include "pruebas/activatebankofproof/activatebankofproof.h"
#include "Arduino.h"
#include "Config.h"
#include <Arduino.h>

// Definiciones


// Variables de control
bool buzzerActive = false;
unsigned long buzzerStartTime = 0;

// Función para activar el buzzer
void beep() {
  if (!buzzerActive) {
    digitalWrite(BUZZER_PIN, HIGH); // Activa el buzzer
    buzzerStartTime = millis();     // Guarda el tiempo de inicio
    buzzerActive = true;            // Marca como activo
  }
}

// Función para actualizar el estado del buzzer (llamar en loop())
void updateBuzzer() {
  if (buzzerActive && (millis() - buzzerStartTime >= 1000)) {
    digitalWrite(BUZZER_PIN, LOW);  // Apaga el buzzer después de 1 segundo
    buzzerActive = false;           // Marca como inactivo
  }
}

void setupbit() {
  pinMode(BUZZER_PIN, OUTPUT);      // Configura el pin como salida
  digitalWrite(BUZZER_PIN, LOW);    // Asegura que el buzzer inicia apagado
}




// Función: controlBomba(activar)
// - activar: true (encender), false (apagar)
void controlBomba(bool activar) {
  if (activar && !bombaActiva) {
    digitalWrite(PIN_BOMBA, HIGH); // Encender bomba
    bombaActiva = true;
    Serial.println("Bomba ACTIVADA");
  } 
  else if (!activar && bombaActiva) {
    digitalWrite(PIN_BOMBA, LOW); // Apagar bomba
    bombaActiva = false;
    Serial.println("Bomba DESACTIVADA");
  }
//   void pruebaCaudal() {
//     controlBomba(true); // Activar bomba para presurizar
//     delay(1000);        // Esperar estabilización
    
//     // Medir flujo durante 10 segundos
//     unsigned long startTime = millis();
//     while (millis() - startTime < 10000) {
//       // ... leer sensor de flujo
//     }
    
//     controlBomba(false); // Apagar bomba
//   }
}

void setupbomba() {
  pinMode(PIN_BOMBA, OUTPUT);
  digitalWrite(PIN_BOMBA, LOW); // Asegurar que inicia apagada
}

// // Inicializar PWM
// void setupPWM() {
//     ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
//     ledcAttachPin(PIN_INYECTOR_PWM, PWM_CHANNEL);
//     ledcWrite(PWM_CHANNEL, 0); // Inicia apagado
//   }
  
  // Función: activarInyector(duracion_ms, duty_cycle)
  // - duracion_ms: Tiempo de activación en milisegundos.
  // - duty_cycle: Porcentaje de apertura (0-100%).
  void activarInyector(unsigned long duracion_ms, float duty_cycle) {
    // Calcular valor del duty cycle (0-255 para 8 bits)
    uint32_t duty = (duty_cycle / 100.0) * 255;
    duty = constrain(duty, 0, 255);
  
    // Encender el inyector
    ledcWrite(PWM_CHANNEL, duty);
    inyectorActivo = true;
    Serial.printf("Inyector ACTIVADO (Duty: %.1f%%)\n", duty_cycle);
  
    // Mantener activo por el tiempo definido
    delay(duracion_ms);
  
    // Apagar el inyector
    ledcWrite(PWM_CHANNEL, 0);
    inyectorActivo = false;
    Serial.println("Inyector DESACTIVADO");
    // void testCaudal() {
    //     // Activar el inyector por 1 segundo al 70% de duty
    //     activarInyector(1000, 70);
    //     // Medir flujo con sensor YF-S201...
    //   }
  }
//prueba de sonido
  void setupmic() {

    pinMode(PIN_SONIDO, INPUT);
    
    // Configurar PWM para el inyector
    ledcSetup(pwmChannell, pwmFreqq, pwmResolutionn);
    ledcAttachPin(PIN_INYECTOR_PWM, pwmChannell);
  }


  // Función para detectar el clic
bool detectarClic() {
    clicDetectado = false;
    unsigned long inicio = millis();
    
    // Muestrear durante TIEMPO_ESPERA ms
    while (millis() - inicio < TIEMPO_ESPERA) {
      int lectura = analogRead(PIN_SONIDO);
      
      // Si supera el umbral, hay clic
      if (lectura > UMBRAL_CLIC) {
        clicDetectado = true;
        break;
      }
    }
    
    return clicDetectado;
  }
  
  // Prueba completa del clic
  void pruebaClic() {
    // Activar inyector (pulso corto)
    ledcWrite(pwmChannell, 255); // 100% duty cycle
    delay(5);                   // Pulso de 5ms
    ledcWrite(pwmChannell, 0);
    
    // Detectar clic
    bool resultado = detectarClic();
    
    // Mostrar resultado
    if (resultado) {
      Serial.println("✔️ Clic detectado - Inyector funcional");
    } else {
      Serial.println("❌ Fallo: No se detectó clic");
    }
  }

void setupi() {

pinMode(INYECTOR_PIN, OUTPUT); // Configuramos el pin como salid

}

void loopi() {
  Serial.println("Inyector activado");
digitalWrite(INYECTOR_PIN, HIGH); // Activamos el inyector
  delay(100); // Mantiene el inyector encendido por 100 ms
  Serial.println("Inyector DESACTIVADO");
  digitalWrite(INYECTOR_PIN, LOW); // Apagamos el inyector
  delay(100); // Mantiene el inyector apagado por 100 ms

}
// Función para activar la bomba