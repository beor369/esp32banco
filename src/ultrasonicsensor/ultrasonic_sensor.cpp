#include "ultrasonic_sensor.h"
#include <map>
#include <vector>

#define ECHO_PIN 2  
#define TRIG_PIN 4  

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


float UltrasonicSensor::cycles_get_distance(int ciclos) {
    std::vector<float> distancias;
    
    for (int ci = 0; ci < ciclos; ci++) {
        float distance = getDistance();
        distancias.push_back(distance);
        delay(1000); // Espera de 1 segundo entre mediciones
    }
    
    // Mapa para contar la frecuencia de cada distancia
    std::map<float, int> frecuencia;
    for (float d : distancias) {
        frecuencia[d]++;
    }

    // Buscar el valor más repetido
    float moda = 0;
    int maxFrecuencia = 0;
    for (const auto& par : frecuencia) {
        if (par.second > maxFrecuencia) {
            maxFrecuencia = par.second;
            moda = par.first;
        }
    }

    return moda; // Retorna el valor más repetido
}