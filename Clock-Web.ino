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

#define UPDATE_INTERVAL 1000

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void show_display( const String mensage ) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println(mensage);
  display.display();
}

void tela_analogica( const int segundos, const int minutos, const int horas, const String horas_minutos ) {

  display.clearDisplay();

  // Desenha o retângulo arredondado 80x64 no meio da tela
  display.drawRoundRect((SCREEN_WIDTH - 80) / 2, 0, 80, 64, 8, SSD1306_WHITE);

  int rectX = (SCREEN_WIDTH - 80) / 2;
  int rectY = 0;


  display.drawLine(rectX + 40, rectY + 2, rectX + 40, rectY + 7, SSD1306_WHITE);
  display.drawLine(rectX + 78, rectY + 32, rectX + 73, rectY + 32, SSD1306_WHITE);
  display.drawLine(rectX + 40, rectY + 62, rectX + 40, rectY + 57, SSD1306_WHITE);
  display.drawLine(rectX + 2, rectY + 32, rectX + 7, rectY + 32, SSD1306_WHITE);

  // Desenha os números
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(rectX + 36, rectY + 10);
  display.print("12");

  display.setCursor(rectX + 65, rectY + 30);
  display.print("3");

  display.setCursor(rectX + 38, rectY + 47);
  display.print("6");

  display.setCursor(rectX + 9, rectY + 29);
  display.print("9");

  display.setCursor(rectX + 26, rectY + 35);
  display.print(horas_minutos);

  // Calcula o ângulo correspondente aos segundos (0 a 59) em radianos
  float segundos_angulo = (segundos - 15) * (PI / 30);
  float minutos_angulo  = ( minutos - 15) * (PI / 30);
  float horas_angulo    = ( horas - 15) * (PI / 6 );

  // Calcula as coordenadas do ponteiro dos segundos
  int centro_segundos_x = (SCREEN_WIDTH - 80) / 2 + 40;
  int centro_segundos_y = 32;
  int ponteiro_segundos_x = centro_segundos_x + int(28 * cos(segundos_angulo));
  int ponteiro_segundos_y = centro_segundos_y + int(28 * sin(segundos_angulo));

  // Calcula as coordenadas do ponteiro dos minutos
  int centro_minutos_x = (SCREEN_WIDTH - 80) / 2 + 40;
  int centro_minutos_y = 32;
  int ponteiro_minutos_x = centro_minutos_x + int(24 * cos(minutos_angulo));
  int ponteiro_minutos_y = centro_minutos_y + int(24 * sin(minutos_angulo));

    // Calcula as coordenadas do ponteiro das horas
  int centro_horas_x = (SCREEN_WIDTH - 80) / 2 + 40;
  int centro_horas_y = 32;
  int ponteiro_horas_x = centro_horas_x + int(20 * cos(horas_angulo));
  int ponteiro_horas_y = centro_horas_y + int(20 * sin(horas_angulo));

  // Desenha o ponteiro dos segundos
  display.drawLine(centro_segundos_x, centro_segundos_y, ponteiro_segundos_x, ponteiro_segundos_y, SSD1306_WHITE);
  // Desenha o ponteiro dos minutos (uma linha reta)
  display.drawLine(centro_minutos_x, centro_minutos_y, ponteiro_minutos_x, ponteiro_minutos_y, SSD1306_WHITE);
  // Desenha o ponteiro das horas (uma linha reta)
  display.drawLine(centro_horas_x, centro_horas_y, ponteiro_horas_x, ponteiro_horas_y, SSD1306_WHITE);


  display.display();
}

void tela_digital( const String format_date, const String time ) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 10);
  display.println(format_date);
  display.setCursor(15, 40);
  display.println(time);
  display.display();
}

void get_horario( const bool trocar ) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(ENDPOINTHOUR);

    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        //Serial.println(payload);

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

        String segundo_string = String(datetime).substring(17, 19);
        String minutos_string = String(datetime).substring(14, 16);
        String horas_string   = String(datetime).substring(11, 13);

        String horas_minutos   = String(datetime).substring(11, 16);

        Serial.print("Hora atual em Brasília: ");
        Serial.println(time);

        // Converte a data para o formato dd/MM/yyyy
        String year = date.substring(0, 4);
        String month = date.substring(5, 7);
        String day = date.substring(8, 10);
        String format_date = day + "/" + month + "/" + year;

        if(trocar) {
          tela_analogica(segundo_string.toInt(), minutos_string.toInt(), (horas_string.toInt() - 12), horas_minutos);
        } else {
          tela_digital(format_date, time);
        }


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
    show_display("Reconectando ao Wi-Fi...");
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);

      Serial.print(".");

    }
    Serial.println();
    Serial.println("Reconectado ao Wi-Fi");
    show_display("Reconectando ao Wi-Fi");
  }
}

void setup() {
  Serial.begin(115200);
  delay(4000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Falha na inicialização do SSD1306");
    for (;;);
  }

  WiFi.begin(SSID, PASSWORD);
  Serial.print("Conectando-se ao Wi-Fi");
  show_display("Conectando-se ao Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Conectado ao Wi-Fi");
  show_display("Conectado ao Wi-Fi");

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(23, 10);
  display.println(F("HORARIO"));
  display.setCursor(15, 40);
  display.println(F("Brasilia"));
  display.display();

  delay(5000);

}

void loop() {

  static unsigned long previous_millis = 0;
  static int trocar_tela = 0;
  static bool trocar = true;

  unsigned long current_millis = millis();

  if (current_millis - previous_millis >= UPDATE_INTERVAL) {
    previous_millis = current_millis;
    get_horario(trocar);

    trocar_tela++;
    if( trocar_tela >= 15 ) {
      trocar_tela = 0;
      trocar = !trocar;
    }
  }

  verifica_conexao();

}
