/*
 * ESP32 Weather Station with DHT11, LCD I2C Display, and Web Dashboard
 * Shows indoor temperature/humidity and outdoor weather from API
 * Access web dashboard at: http://[ESP32_IP_ADDRESS]
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "WIFI";
const char* password = "PASSWORD";

// OpenWeatherMap API settings
const char* apiKey = "APIKEY";  // Get free key from openweathermap.org
const char* city = "Giza";  // Change to your city
const char* countryCode = "EG";  // Two-letter country code

// DHT11 sensor setup
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Web server on port 80
WebServer server(80);

// Timing variables
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherUpdateInterval = 600000;
unsigned long lastDHTRead = 0;
const unsigned long dhtReadInterval = 2000;

// Weather data variables
float outdoorTemp = 0;
int outdoorHumidity = 0;
String weatherDescription = "";
float indoorTemp = 0;
float indoorHumidity = 0;
String lastUpdateTime = "";

void setup() {
  Serial.begin(115200);
  
  dht.begin();
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Weather Station");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(3000);
    
    // Setup web server routes
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();
    Serial.println("Web server started");
    
    getWeatherData();
  } else {
    Serial.println("\nWiFi Connection Failed!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
  }
}

void loop() {
  // Handle web server requests
  server.handleClient();
  
  // Read DHT11 sensor
  if (millis() - lastDHTRead >= dhtReadInterval) {
    lastDHTRead = millis();
    indoorTemp = dht.readTemperature();
    indoorHumidity = dht.readHumidity();
    
    if (isnan(indoorTemp) || isnan(indoorHumidity)) {
      Serial.println("Failed to read from DHT sensor!");
    }
  }
  
  // Update outdoor weather data
  if (millis() - lastWeatherUpdate >= weatherUpdateInterval) {
    lastWeatherUpdate = millis();
    if (WiFi.status() == WL_CONNECTED) {
      getWeatherData();
    }
  }
  
  // Update LCD display
  updateDisplay();
  
  delay(2000);
}

void getWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + 
                 String(city) + "," + String(countryCode) + 
                 "&units=metric&appid=" + String(apiKey);
    
    Serial.println("Fetching weather data...");
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Weather data received");
      
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        outdoorTemp = doc["main"]["temp"];
        outdoorHumidity = doc["main"]["humidity"];
        weatherDescription = doc["weather"][0]["main"].as<String>();
        
        // Get current time (minutes since boot as simple timestamp)
        unsigned long minutes = millis() / 60000;
        lastUpdateTime = String(minutes) + " min ago";
        
        Serial.print("Outdoor Temp: ");
        Serial.println(outdoorTemp);
        Serial.print("Description: ");
        Serial.println(weatherDescription);
      } else {
        Serial.println("JSON parsing failed!");
      }
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(httpCode);
    }
    
    http.end();
  }
}

void updateDisplay() {
  lcd.clear();
  
  // Line 1: Indoor conditions
  lcd.setCursor(0, 0);
  lcd.print("In:");
  if (!isnan(indoorTemp)) {
    lcd.print(indoorTemp, 1);
    lcd.print("C ");
  } else {
    lcd.print("--C ");
  }
  
  if (!isnan(indoorHumidity)) {
    lcd.print((int)indoorHumidity);
    lcd.print("%");
  } else {
    lcd.print("--%");
  }
  
  // Line 2: Outdoor conditions
  lcd.setCursor(0, 1);
  lcd.print("Out:");
  if (outdoorTemp != 0) {
    lcd.print(outdoorTemp, 1);
    lcd.print("C ");
    
    String desc = weatherDescription;
    if (desc.length() > 5) {
      desc = desc.substring(0, 5);
    }
    lcd.print(desc);
  } else {
    lcd.print("Loading...");
  }
}

// Handle root web page
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta charset='UTF-8'>";
  html += "<title>Weather Station Dashboard</title>";
  html += "<style>";
  html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
  html += "body { font-family: 'Segoe UI', Arial, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); ";
  html += "min-height: 100vh; display: flex; justify-content: center; align-items: center; padding: 20px; }";
  html += ".container { max-width: 800px; width: 100%; }";
  html += ".header { text-align: center; color: white; margin-bottom: 30px; }";
  html += ".header h1 { font-size: 2.5em; margin-bottom: 10px; text-shadow: 2px 2px 4px rgba(0,0,0,0.3); }";
  html += ".header p { font-size: 1.1em; opacity: 0.9; }";
  html += ".cards { display: grid; grid-template-columns: repeat(auto-fit, minmax(350px, 1fr)); gap: 20px; }";
  html += ".card { background: white; border-radius: 20px; padding: 30px; box-shadow: 0 10px 30px rgba(0,0,0,0.3); }";
  html += ".card-title { font-size: 1.3em; color: #667eea; margin-bottom: 20px; font-weight: 600; }";
  html += ".metric { display: flex; justify-content: space-between; align-items: center; margin: 15px 0; padding: 15px; ";
  html += "background: #f8f9fa; border-radius: 10px; }";
  html += ".metric-label { font-size: 1.1em; color: #666; }";
  html += ".metric-value { font-size: 2em; font-weight: bold; color: #333; }";
  html += ".temp { color: #ff6b6b; }";
  html += ".humidity { color: #4ecdc4; }";
  html += ".weather-desc { text-align: center; font-size: 1.5em; color: #667eea; margin: 20px 0; ";
  html += "padding: 15px; background: #f0f0f0; border-radius: 10px; font-weight: 600; }";
  html += ".update-info { text-align: center; color: #999; margin-top: 20px; font-size: 0.9em; }";
  html += ".loading { text-align: center; color: white; font-size: 1.2em; }";
  html += "@media (max-width: 768px) { .cards { grid-template-columns: 1fr; } .header h1 { font-size: 2em; } }";
  html += "</style>";
  html += "<script>";
  html += "function updateData() {";
  html += "  fetch('/data').then(r => r.json()).then(data => {";
  html += "    document.getElementById('inTemp').textContent = data.indoorTemp + '°C';";
  html += "    document.getElementById('inHum').textContent = data.indoorHumidity + '%';";
  html += "    document.getElementById('outTemp').textContent = data.outdoorTemp + '°C';";
  html += "    document.getElementById('outHum').textContent = data.outdoorHumidity + '%';";
  html += "    document.getElementById('weather').textContent = data.weatherDescription;";
  html += "    document.getElementById('update').textContent = 'Last updated: ' + data.lastUpdate;";
  html += "  }).catch(err => console.error('Error:', err));";
  html += "}";
  html += "setInterval(updateData, 2000);";
  html += "window.onload = updateData;";
  html += "</script>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<div class='header'>";
  html += "<h1>🌤️ Weather Station</h1>";
  html += "<p>Real-time Indoor & Outdoor Monitoring</p>";
  html += "</div>";
  html += "<div class='cards'>";
  
  // Indoor card
  html += "<div class='card'>";
  html += "<div class='card-title'>🏠 Indoor Climate</div>";
  html += "<div class='metric'>";
  html += "<span class='metric-label'>Temperature</span>";
  html += "<span class='metric-value temp' id='inTemp'>--°C</span>";
  html += "</div>";
  html += "<div class='metric'>";
  html += "<span class='metric-label'>Humidity</span>";
  html += "<span class='metric-value humidity' id='inHum'>--%</span>";
  html += "</div>";
  html += "</div>";
  
  // Outdoor card
  html += "<div class='card'>";
  html += "<div class='card-title'>🌍 Outdoor Weather</div>";
  html += "<div class='weather-desc' id='weather'>Loading...</div>";
  html += "<div class='metric'>";
  html += "<span class='metric-label'>Temperature</span>";
  html += "<span class='metric-value temp' id='outTemp'>--°C</span>";
  html += "</div>";
  html += "<div class='metric'>";
  html += "<span class='metric-label'>Humidity</span>";
  html += "<span class='metric-value humidity' id='outHum'>--%</span>";
  html += "</div>";
  html += "</div>";
  
  html += "</div>";
  html += "<div class='update-info' id='update'>Loading...</div>";
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// Handle data endpoint (JSON)
void handleData() {
  JsonDocument doc;
  
  doc["indoorTemp"] = !isnan(indoorTemp) ? String(indoorTemp, 1) : "--";
  doc["indoorHumidity"] = !isnan(indoorHumidity) ? String((int)indoorHumidity) : "--";
  doc["outdoorTemp"] = outdoorTemp != 0 ? String(outdoorTemp, 1) : "--";
  doc["outdoorHumidity"] = outdoorHumidity != 0 ? String(outdoorHumidity) : "--";
  doc["weatherDescription"] = weatherDescription.length() > 0 ? weatherDescription : "Loading...";
  doc["lastUpdate"] = lastUpdateTime.length() > 0 ? lastUpdateTime : "Starting...";
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  server.send(200, "application/json", jsonString);
}