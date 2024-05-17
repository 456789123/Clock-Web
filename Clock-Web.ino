#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define SSID         "---"
#define PASSWORD     "---"
#define ENDPOINTHOUR "http://worldtimeapi.org/api/timezone/America/Sao_Paulo"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define UPDATE_INTERVAL 1000

void get_horario() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(ENDPOINTHOUR);

    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
          Serial.print("Erro ao fazer o parse do JSON: ");
          Serial.println(error.c_str());
          return;
        }

        // Extrai a hora atual no formato 24 horas
        const char* datetime = doc["datetime"];
        String time = String(datetime).substring(11, 19);
        String date = String(datetime).substring(0, 10);
        Serial.print("Hora atual em Brasília: ");
        Serial.println(time);

        // Converte a data para o formato dd/MM/yyyy
        String year = date.substring(0, 4);
        String month = date.substring(5, 7);
        String day = date.substring(8, 10);
        String formattedDate = day + "/" + month + "/" + year;

        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(5, 10);
        display.println(formattedDate);
        display.setCursor(15, 40);
        display.println(time);
        display.display();

      } else {
        Serial.print("Código de resposta HTTP: ");
        Serial.println(httpCode);
      }
    } else {
      Serial.println("Erro na requisição HTTP");
    }
    http.end();
  } else {
    Serial.println("WiFi desconectado");
  }
}

void verifica_conexao() {
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconectando ao Wi-Fi...");
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
    Serial.println("Reconectado ao Wi-Fi");
  }
}

void setup() {
  Serial.begin(115200);
  delay(4000);

  WiFi.begin(SSID, PASSWORD);
  Serial.print("Conectando-se ao Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Conectado ao Wi-Fi");

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Falha na inicialização do SSD1306");
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 10);
  display.println(F("HORARIO"));
  display.setCursor(10, 30);
  display.println(F("Brasilia"));
  display.display();

  delay(5000);
}

void loop() {
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= UPDATE_INTERVAL) {
    previousMillis = currentMillis;
    get_horario();
  }

  verifica_conexao();
}
