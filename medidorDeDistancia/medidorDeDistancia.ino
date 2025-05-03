#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "SCHARSS";
const char* password = "31142731";

const int trigPin = 5;  // Cambia según el pin del Trig
const int echoPin = 18; // Cambia según el pin del Echo

WebServer server(80);

String paginaHTML;

float distancia;

// Configuración de WiFi y página HTML
void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData); // Añadimos la ruta "/data" para enviar los datos
  server.begin();
}

// Función para medir la distancia
float medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;
  return distance;
}

// Genera el HTML con la gráfica
void handleRoot() {
  paginaHTML = "<!DOCTYPE html><html lang='es'>";
  paginaHTML += "<head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  paginaHTML += "<title>Medición de Distancia</title>";
  paginaHTML += "<link href='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css' rel='stylesheet'>";
  paginaHTML += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script></head>";
  paginaHTML += "<body><div class='container mt-4'><h2>Medición de Distancia en Tiempo Real</h2>";
  paginaHTML += "<canvas id='distanciaChart'></canvas></div>";
  paginaHTML += "<script>";
  paginaHTML += "let data = []; let labels = [];";
  paginaHTML += "function actualizarDatos() { fetch('/data').then(response => response.json()).then(dist => {";
  paginaHTML += "if (data.length > 20) { data.shift(); labels.shift(); }";
  paginaHTML += "data.push(dist.distancia); labels.push('');";
  paginaHTML += "chart.update(); }); }";
  paginaHTML += "const ctx = document.getElementById('distanciaChart').getContext('2d');";
  paginaHTML += "const chart = new Chart(ctx, { type: 'line', data: { labels: labels, datasets: [{ label: 'Distancia (cm)', data: data, borderColor: 'rgba(75, 192, 192, 1)', borderWidth: 1, fill: false }] }, options: { scales: { y: { beginAtZero: true } } } });";
  paginaHTML += "setInterval(actualizarDatos, 1000);";
  paginaHTML += "</script></body></html>";

  server.send(200, "text/html", paginaHTML);
}

// Enviar la distancia en formato JSON
void handleData() {
  distancia = medirDistancia();
  String jsonData = "{\"distancia\":" + String(distancia) + "}";
  server.send(200, "application/json", jsonData);
}

void loop() {
  server.handleClient();
}
