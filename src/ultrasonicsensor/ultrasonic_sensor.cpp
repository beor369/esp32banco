#include "ultrasonic_sensor.h"

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
