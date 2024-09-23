#include <Arduino.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

// Ustawienia pinu i typu czujnika
#define DHTPIN D7
#define DHTTYPE DHT22

// Inicjalizacja czujnika DHT
DHT dht(DHTPIN, DHTTYPE);

// Dane sieci Wi-Fi
const char* ssid = "PLAY internet 2.4GHz_6B1D";
const char* password = "jagodowa";

// Adres URL serwera Spring Boot
const char* serverUrl = "http://192.168.1.3:8080/iot/data";

// ID urządzenia
const char* deviceId = "ESP8266-temperature-sensor";

void setup() {
  // Inicjalizacja komunikacji szeregowej
  Serial.begin(115200);
  
  // Inicjalizacja czujnika
  dht.begin();

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
  if (WiFi.status() == WL_CONNECTED) {
    // Odczyt temperatury
    float temperature = dht.readTemperature();
    
    // Sprawdzenie, czy odczyt się powiódł
    if (isnan(temperature)) {
      Serial.println("Błąd odczytu temperatury!");
      return;
    }

    // Wyświetlenie temperatury w Serial Monitorze
    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.println(" *C");

    // Wysyłanie danych do serwera
    HTTPClient http;
    WiFiClient client; // Dodaj obiekt WiFiClient
    // Otwórz połączenie z serwerem
    http.begin(client, serverUrl); // Użyj WiFiClient i URL
    http.addHeader("Content-Type", "application/json");

    // Przygotowanie danych w formacie JSON
    String jsonData = "{\"deviceId\":\"" + String(deviceId) +
                      "\", \"temperatureSensor\":" + String(temperature) + "}";
    Serial.println(jsonData); 
     
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
  } else {
    Serial.println("Brak połączenia z Wi-Fi");
  }

  // Opóźnienie co 10 sekund
  delay(10000);
}