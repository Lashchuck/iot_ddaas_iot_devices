#include <Arduino.h>
#include <HTTPClient.h>

// Definicje pinów do czujników wilgotności gleby
#define SENSOR_PIN_1 32
#define SENSOR_PIN_2 33

// Dane sieci Wi-Fi
const char* ssid = "PLAY internet 2.4GHz_6B1D";
const char* password = "jagodowa";

// Adres URL serwera Spring Boot
const char* serverUrl = "http://192.168.1.3:8080/iot/data";

// ID urządzenia
const char* deviceId = "ESP-32-moisture-sensors";

// Funkcja do skalibrowania odczytów z czujnika na procenty
int calibrateSensorValue(int rawValue) {
  // Kalibracja liniowa
  int calibratedValue = map(rawValue, 2286, 4095, 100, 0);
  // Zapewnienie, że wartości mieszczą się w zakresie 0-100%
  calibratedValue = constrain(calibratedValue, 0, 100);
  return calibratedValue;
}

void setup() {
  // Rozpoczęcie komunikacji szeregowej
  Serial.begin(115200);

  // Ustawienie pinów jako wejścia
  pinMode(SENSOR_PIN_1, INPUT);
  pinMode(SENSOR_PIN_2, INPUT);

  // Połączenie z Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Łączenie z Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Połączono");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Odczyt wartości z czujników wilgotności
  int rawValue1 = analogRead(SENSOR_PIN_1);
  int rawValue2 = analogRead(SENSOR_PIN_2);

  // Kalibracja odczytów na procenty
  int moisturePercent1 = calibrateSensorValue(rawValue1);
  int moisturePercent2 = calibrateSensorValue(rawValue2);

  // Wyświetlenie wartości w monitorze portu szeregowego
  Serial.print("Wilgotność czujnika 1: ");
  Serial.print(moisturePercent1);
  Serial.println("%");
  
  Serial.print("Wilgotność czujnika 2: ");
  Serial.print(moisturePercent2);
  Serial.println("%");
  
  // Wysyłanie danych do serwera
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    // Otwórz połączenie z serwerem
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    // Przygotowanie danych w formacie JSON
    String jsonData = "{\"deviceId\":\"" + String(deviceId) +
                      "\", \"sensor1\":" + String(moisturePercent1) +
                      ", \"sensor2\":" + String(moisturePercent2) + "}";
  

    // Wykonanie żądania POST
    int httpResponseCode = http.POST(jsonData);

    // Sprawdzenie odpowiedzi serwera
    if(httpResponseCode > 0){
      Serial.print("Odpowiedź serwera: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
      Serial.println(response);
    }else{
      Serial.print("Błąd podczas wysyłania POST: ");
      Serial.println(httpResponseCode);
    }

    // Zakończenie połączenia
    http.end();
  }else{
    Serial.println("Brak połączenia z Wi-Fi");
  }
  
  // Opóźnienie co 5 sekund
  delay(5000);
}
