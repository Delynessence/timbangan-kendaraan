#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

#define DOUT D5
#define CLK D6
#define BUZZER_PIN D7

HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80); // Use HTTP port 80 for web server

const char* ssid = "Magister";
const char* password = "R!$np4n1viz";
const char* googleScriptUrlWeight = "https://script.google.com/macros/s/AKfycbwQxCU4k1fJWlJ2RCM1B-jAo3nb1v3x9T0j4TXIYFweje8rYirzPKMba-v69v0hXh3f/exec"; // Real-time weight

String vehicleType = "";
float currentWeight = 0;
float maxWeight = 0;
float vehicleWeight = 0; 
float calibrationFactor = 767964.63; // Hasil kalibrasi

// Variabel berat kendaraan dan max muatan untuk jenis kendaraan
struct Vehicle {
  String name;
  float maxWeight;
  float vehicleWeight;
};

Vehicle vehicles[] = {
  {"Truk", 10000, 5000},
  {"Mobil", 2000, 1200},
  {"Motor", 500, 150}
};

void sendDataToGoogleSheet(float netWeight) {
  WiFiClientSecure client;
  client.setInsecure(); // Ini digunakan untuk koneksi yang tidak aman

  String url = String(googleScriptUrlWeight) + "?weight=" + String(netWeight);
  if (client.connect("script.google.com", 443)) {
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: script.google.com\r\n" +
                 "Connection: close\r\n\r\n");
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);
  
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(0, 1);
    lcd.print(".");
    delay(1000);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  delay(2000);
  
  scale.begin(DOUT, CLK);
  scale.set_scale(calibrationFactor);
  scale.tare(); 
  
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Handler untuk menerima jenis kendaraan dari website
  server.on("/setVehicleType", HTTP_GET, []() {
    vehicleType = server.arg("vehicleType");
    bool vehicleFound = false;

    // Mencari jenis kendaraan yang cocok dan menetapkan berat serta maxWeight
    for (Vehicle v : vehicles) {
      if (v.name == vehicleType) {
        maxWeight = v.maxWeight;
        vehicleWeight = v.vehicleWeight;
        vehicleFound = true;
        break;
      }
    }

    if (vehicleFound) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Type: " + vehicleType);
      lcd.setCursor(0, 1);
      lcd.print("MaxW: " + String(maxWeight) + " Kg");
      server.send(200, "text/plain", "Vehicle type set to: " + vehicleType);
    } else {
      server.send(400, "text/plain", "Invalid vehicle type.");
    }
  });

  // Endpoint untuk menampilkan berat real-time di website
  server.on("/getWeight", HTTP_GET, []() {
    String response = String(currentWeight);
    server.send(200, "text/plain", response);
  });

  server.begin();
}

void loop() {
  server.handleClient();

  if (scale.is_ready()) {
    currentWeight = scale.get_units(10);
    if (currentWeight < 0) currentWeight = 0;

    float netWeight = currentWeight - vehicleWeight; // Berat bersih
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Weight: ");
    lcd.print(netWeight);
    lcd.print(" Kg");

    sendDataToGoogleSheet(netWeight); // Kirim data berat ke Google Sheet

    // Cek jika berat bersih melebihi maxWeight
    if (netWeight > maxWeight) {
      lcd.setCursor(0, 1);
      lcd.print("Overload!");
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Normal");
      digitalWrite(BUZZER_PIN, LOW);
    }

    delay(500);
  }
}
