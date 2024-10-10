#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>  // Make sure to include this for JSON parsing

// Inisialisasi komponen
#define DOUT D5
#define CLK D6
#define BUZZER_PIN D7

HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80); // Change to port 80 for easier access

// Ganti dengan SSID dan Password WiFi Anda
const char* ssid = "Magister";
const char* password = "R!$np4n1viz";

// Ganti dengan ID script Google Apps Anda
const char* googleScriptUrl = "https://script.google.com/macros/s/AKfycbw7XOIB4kGaH1tgXyqa1vMlRUzlv0HoEQiO45o6hovSHovHVxu5Wy2sIbJkq75n0cWIQg/exec"; // URL untuk fetch vehicle type and weight

String vehicleType = "";
float currentWeight = 0;
float maxWeight = 0;  
float vehicleWeight = 0;  
float calibrationFactor =  767964.63;  // Hasil dari perhitungan kalibrasi

void sendDataToGoogleSheet(float currentWeight) {
    // Implement sending data to the Google Sheets if needed
}

void getVehicleDataFromGoogleSheet() {
    WiFiClientSecure client;
    client.setInsecure();

    // Construct the URL with vehicle type
    String vehicleDataUrl = String(googleScriptUrl) + "?vehicleType=" + vehicleType;
    if (client.connect("script.google.com", 443)) {
        client.print(String("GET ") + vehicleDataUrl + " HTTP/1.1\r\n" +
                     "Host: script.google.com\r\n" +
                     "Connection: close\r\n\r\n");

        String line = "";
        while (client.available()) {
            line = client.readStringUntil('\n');
        }

        // Parse JSON response
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, line);

        vehicleType = doc["vehicleType"].as<String>(); // Get the selected vehicle type
        maxWeight = doc["maxWeight"];  // Get the maximum weight for this vehicle

        // Print to Serial Monitor for debugging
        Serial.print("Vehicle type: ");
        Serial.println(vehicleType);
        Serial.print("Max weight: ");
        Serial.println(maxWeight);
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
    scale.set_scale(calibrationFactor);  // Menggunakan faktor kalibrasi langsung
    scale.tare();  // Tare the scale to start from 0 after calibration
    
    pinMode(BUZZER_PIN, OUTPUT);
    
    server.on("/setVehicleType", HTTP_GET, []() {
        vehicleType = server.arg("vehicleType");
        getVehicleDataFromGoogleSheet(); // Fetch vehicle type data
        server.send(200, "text/plain", "Vehicle type set: " + vehicleType);
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
        currentWeight = scale.get_units(10);  // Faktor kalibrasi sudah diterapkan
        if (currentWeight < 0) currentWeight = 0;

        float netWeight = currentWeight - vehicleWeight;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Weight: ");
        lcd.print(netWeight);
        lcd.print(" Kg");

        // Check if netWeight exceeds maxWeight
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
