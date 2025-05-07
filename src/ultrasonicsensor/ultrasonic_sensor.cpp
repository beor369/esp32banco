#include "ultrasonic_sensor.h"
#include <map>
#include <vector>

#define ECHO_PIN 4  
#define TRIG_PIN 2  

UltrasonicSensor ultrasonic_sensor(TRIG_PIN, ECHO_PIN);

UltrasonicSensor::UltrasonicSensor(int trigPin, int echoPin) 
    : trigPin(trigPin), echoPin(echoPin) {}

void UltrasonicSensor::begin() {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
}

float UltrasonicSensor::getDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH);
    return duration * 0.034 / 2; // Convertir a cm
}

void UltrasonicSensor::printDistance() {
    float distance = getDistance();
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
}

// Función mejorada de UltrasonicSensor para usar mediana
// Función mejorada de UltrasonicSensor para usar mediana

// Función mejorada de UltrasonicSensor para usar mediana y descartar outliers extremos
float UltrasonicSensor::get_distance_fashion(int ciclos, bool saveMedian) {
    const float MAX_DISTANCE_CM = 500.0f;
    const float MIN_DISTANCE_CM =  2.0f;
    std::vector<float> distancias;
    distancias.reserve(ciclos);
  
    for (int i = 0; i < ciclos; i++) {
      float d = getDistance();
      // Filtrar lecturas inválidas
      if (d >= MIN_DISTANCE_CM && d <= MAX_DISTANCE_CM) {
        distancias.push_back(d);
      }
      delay(200);
    }
    if (distancias.empty()) {
      return saveMedian ? this->distaceFashion : -1.0f;
    }
    // Calcular mediana
    size_t mid = distancias.size() / 2;
    std::nth_element(distancias.begin(), distancias.begin() + mid, distancias.end());
    float median = distancias[mid];
    if (saveMedian) this->distaceFashion = median;
    return median;
  }
  