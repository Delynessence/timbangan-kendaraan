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
ESP8266WebServer server(80);  // Ubah ke port 80 untuk HTTP

const char* ssid = "Magister";
const char* password = "R!$np4n1viz";

String vehicleType = "Unknown";
float currentWeight = 0;
float maxWeight = 0;  
float vehicleWeight = 0;  
float calibrationFactor = 767964.63;

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

  server.on("/setVehicleType", HTTP_GET, []() {
    if (server.hasArg("type")) {
      vehicleType = server.arg("type");
      if (vehicleType == "Car") {
        maxWeight = 1000;
      } else if (vehicleType == "Truck") {
        maxWeight = 5000;
      } else if (vehicleType == "Bike") {
        maxWeight = 150;
      } else {
        vehicleType = "Unknown";
        maxWeight = 0;
      }
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Veh: " + vehicleType);
      lcd.setCursor(0, 1);
      lcd.print("Max: " + String(maxWeight) + " Kg");
      server.send(200, "text/plain", "Vehicle: " + vehicleType + ", Max Weight: " + String(maxWeight));
    } else {
      server.send(400, "text/plain", "Vehicle type not provided");
    }
  });

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

    float netWeight = currentWeight - vehicleWeight;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Weight: ");
    lcd.print(netWeight);
    lcd.print(" Kg");

    // Kirim data ke website untuk real-time monitoring
    // Tidak perlu koneksi baru ke Google Script
    if (netWeight > maxWeight) {
      lcd.setCursor(0, 1);
      lcd.print("Overload!");
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Normal");
      digitalWrite(BUZZER_PIN, LOW);
    }

    delay(200);
  }
}
