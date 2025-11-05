#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <HTTPClient.h>

// --------------------- CONFIGURAÇÕES ---------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_MPU6050 mpu;

// WiFi & ThingSpeak
const char* ssid = "Wokwi-GUEST";           // Troque pelo seu WiFi
const char* password = "";      // Troque pela sua senha
const String apiKey = "NTJGYSC9J0VQYVWN"; // API Key ThingSpeak
const String thingSpeakURL = "http://api.thingspeak.com/update";

// --------------------- VARIÁVEIS ---------------------
float distance = 0; // Distância percorrida (km)
float x = 0, y = 0; // Coordenadas simuladas
unsigned long previousMillis = 0;
const long interval = 5000; // Envio de dados a cada 5s

// --------------------- SETUP ---------------------
void setup() {
  Serial.begin(115200);

  // Inicializa display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 falhou!");
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Inicializa MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 não encontrado!");
    while (1) delay(10);
  }
  Serial.println("MPU6050 pronto!");

  // Conecta ao WiFi com timeout
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("\nWiFi conectado!");
  } else {
    Serial.println("\nFalha na conexão WiFi, continuando sem conexão...");
  }
}

// --------------------- LOOP ---------------------
void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Calcula magnitude da aceleração (simulação)
  float acc = sqrt(a.acceleration.x * a.acceleration.x +
                   a.acceleration.y * a.acceleration.y);

  // Atualiza distância percorrida (simulada)
  distance += acc * 0.00005;

  // Atualiza coordenadas simuladas
  x += a.acceleration.x * 0.0001;
  y += a.acceleration.y * 0.0001;

  // Atualiza display OLED
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Distancia: ");
  display.print(distance, 2);
  display.println(" km");
  display.print("X: "); display.println(x, 2);
  display.print("Y: "); display.println(y, 2);
  display.display();

  // Envia dados para ThingSpeak a cada intervalo
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sendToThingSpeak(distance, x, y);
  }

  delay(100);
}

// --------------------- FUNÇÃO DE ENVIO ---------------------
void sendToThingSpeak(float distance, float x, float y) {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = thingSpeakURL + "?api_key=" + apiKey +
                 "&field1=" + String(distance, 2) +
                 "&field2=" + String(x, 2) +
                 "&field3=" + String(y, 2);
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.println("Dados enviados! Código: " + String(httpResponseCode));
    } else {
      Serial.println("Erro no envio: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi desconectado, não foi possível enviar dados!");
  }
}