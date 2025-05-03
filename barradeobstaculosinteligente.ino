#include <ESP32Servo.h>

// Pines del sensor ultrasónico
const int trigPin = 27; // Pin TRIG del HC-SR04
const int echoPin = 26; // Pin ECHO del HC-SR04

// Pines para el LED y el servomotor
const int ledPin = 25;  // Pin del LED
Servo servoMotor;       // Objeto para el servomotor

// Constantes
const int distanciaActivacion = 10; // Distancia en cm para activar el sistema
const int anguloAbierto = 90;       // Ángulo para la posición abierta
const int anguloCerrado = 0;        // Ángulo para la posición cerrada
const int tiempoEspera = 5000;      // Tiempo en ms para mantener la barrera cerrada

void setup() {
  // Configuración de pines
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  servoMotor.attach(14); // Pin del servomotor

  // Inicializar estado
  digitalWrite(ledPin, LOW);
  servoMotor.write(anguloAbierto); // La barrera comienza abierta

  // Inicializar serial para depuración
  Serial.begin(115200);
}

void loop() {
  // Leer la distancia del sensor ultrasónico
  long distancia = medirDistancia();
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  // Activar el sistema si se detecta un objeto cercano
  if (distancia > 0 && distancia <= distanciaActivacion) {
    servoMotor.write(anguloCerrado);     // Cerrar la barrera
    digitalWrite(ledPin, HIGH);          // Encender el LED
    delay(tiempoEspera);                 // Mantener la barrera cerrada por un tiempo
    servoMotor.write(anguloAbierto);     // Reabrir la barrera
    digitalWrite(ledPin, LOW);           // Apagar el LED
  }

  delay(200); // Breve retraso para estabilizar lecturas
}

// Función para medir la distancia con el HC-SR04
long medirDistancia() {
  // Enviar pulso TRIG
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Leer tiempo de respuesta del ECHO
  long duracion = pulseIn(echoPin, HIGH);

  // Convertir tiempo a distancia en cm
  long distancia = duracion * 0.034 / 2;

  return distancia;
}
  