#include <Wire.h> // Librería para comunicación I2C
#include <Adafruit_GFX.h> // Librería base para gráficos
#include <Adafruit_SSD1306.h> // Librería específica para el driver SSD1306

// Pines para el sensor HC-SR04
const int trigPin = 2; // Pin digital conectado al Trig del HC-SR04
const int echoPin = 4; // Pin digital conectado al Echo del HC-SR04

// Dimensiones de la pantalla OLED
#define SCREEN_WIDTH 128 // Ancho en píxeles (común para 128x64 o 128x32)
#define SCREEN_HEIGHT 64 // Alto en píxeles (verifica si es 64 o 32)

// Dirección I2C de la pantalla OLED
// La más común es 0x3C o 0x3D. Si no sabes cuál es, puedes ejecutar un scanner I2C
#define OLED_RESET    -1 // Reset pin # (o -1 si no se usa)
#define SCREEN_ADDRESS 0x3C // Dirección I2C común, prueba 0x3D si 0x3C no funciona

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200); // Iniciar comunicación serial para depuración

  // Configurar pines del sensor ultrasónico
  pinMode(trigPin, OUTPUT); // El pin Trig es de salida
  pinMode(echoPin, INPUT);  // El pin Echo es de entrada

  // Inicializar la pantalla OLED con la dirección I2C especificada
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Error: No se pudo inicializar la pantalla SSD1306. Verifica el cableado y la dirección I2C."));
    for(;;); // Bucle infinito, no continúa
  }

  Serial.println(F("Pantalla OLED inicializada."));

  // Mostrar un mensaje inicial en la pantalla
  display.clearDisplay(); // Limpiar el buffer de la pantalla
  display.setTextSize(1); // Tamaño del texto (1 es el más pequeño)
  display.setTextColor(SSD1306_WHITE); // Color del texto (blanco en una pantalla monocromática)
  display.setCursor(0, 0); // Posición del cursor (columna, fila)
  display.println("Iniciando...");
  display.display(); // Muestra el contenido del buffer en la pantalla

  delay(2000); // Esperar 2 segundos
}

void loop() {
  long duration; // Variable para almacenar la duración del pulso de eco
  int distanceCm; // Variable para almacenar la distancia en centímetros

  // 1. Generar un pulso de 10us en el pin Trig
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // Pulso de 10 microsegundos
  digitalWrite(trigPin, LOW);

  // 2. Medir la duración del pulso de eco en el pin Echo
  // pulseIn() espera a que el pin Echo cambie a HIGH, mide cuánto tiempo permanece HIGH, y devuelve la duración en microsegundos.
  duration = pulseIn(echoPin, HIGH);

  // 3. Calcular la distancia
  // La velocidad del sonido es aproximadamente 343 metros/segundo a 20°C.
  // Esto es 0.0343 cm/microsegundo o 1 cm cada 29.1 microsegundos.
  // La distancia medida es la mitad del tiempo de vuelo (ida y vuelta).
  // Distancia (cm) = (Duración del pulso en microsegundos / 2) * Velocidad del sonido (cm/us)
  // Distancia (cm) = Duración / 58.2 (aprox)
  distanceCm = duration / 58; // División entera para cm, 58 es una aproximación común para cm

  // Limitar el rango de medición para evitar valores erróneos
  if (distanceCm > 400) { // HC-SR04 típicamente tiene un rango máximo de unos 400 cm
    distanceCm = 401; // Usamos un valor para indicar "fuera de rango"
  }
  if (distanceCm < 0) { // Puede dar valores negativos si no detecta eco
     distanceCm = 0; // Usamos 0 para indicar "muy cerca o error"
  }


  // 4. Mostrar la distancia en la pantalla OLED
  display.clearDisplay(); // Limpiar el buffer para la nueva lectura

  display.setTextSize(2); // Texto más grande para la distancia
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Distancia:"); // Etiqueta

  display.setCursor(0, 20); // Posición para la distancia

  if (distanceCm == 401) {
      display.println("Fuera de rango");
  } else if (distanceCm == 0) {
      display.println("Muy cerca");
  }
  else {
      display.print(distanceCm); // Mostrar el valor de la distancia
      display.println(" cm"); // Mostrar las unidades
  }


  display.display(); // Muestra el contenido del buffer en la pantalla

  // También puedes imprimir en el Serial Monitor para depuración
  Serial.print("Distancia: ");
  Serial.print(distanceCm);
  Serial.println(" cm");


  delay(500); // Esperar medio segundo antes de tomar la siguiente lectura
}