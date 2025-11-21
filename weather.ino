#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// OpenWeatherMap API settings
const char* apiKey = "YOUR_API_KEY";  // API key from openweathermap.org
const char* city = "Giza";
const char* countryCode = "EG";

// DHT11 sensor setup
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// Timing variables
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherUpdateInterval = 600000;  // Update every 10 minutes
unsigned long lastDHTRead = 0;
const unsigned long dhtReadInterval = 2000;  // Read DHT every 2 seconds

// Weather data variables
float outdoorTemp = 0;
int outdoorHumidity = 0;
String weatherDescription = "";
float indoorTemp = 0;
float indoorHumidity = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize DHT sensor
  dht.begin();
  
  // Initialize LCD
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
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    delay(2000);
    
    // Get initial weather data
    getWeatherData();
  } else {
    Serial.println("\nWiFi Connection Failed!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
  }
}

void loop() {
  // Read DHT11 sensor
  if (millis() - lastDHTRead >= dhtReadInterval) {
    lastDHTRead = millis();
    indoorTemp = dht.readTemperature();
    indoorHumidity = dht.readHumidity();
    
    // Check if readings are valid
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
  
  delay(5000);  // Refresh display every 5 seconds
}

void getWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Build API URL
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + 
                 String(city) + "," + String(countryCode) + 
                 "&units=metric&appid=" + String(apiKey);
    
    Serial.println("Fetching weather data...");
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Weather data received");
      
      // Parse JSON
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        outdoorTemp = doc["main"]["temp"];
        outdoorHumidity = doc["main"]["humidity"];
        weatherDescription = doc["weather"][0]["main"].as<String>();
        
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
  
  // LCD Line 1: Indoor conditions
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
  
  // LCD Line 2: Outdoor conditions
  lcd.setCursor(0, 1);
  lcd.print("Out:");
  if (outdoorTemp != 0) {
    lcd.print(outdoorTemp, 1);
    lcd.print("C ");
    
    // Show weather description
    String desc = weatherDescription;
    if (desc.length() > 5) {
      desc = desc.substring(0, 5);
    }
    lcd.print(desc);
  } else {
    lcd.print("Loading...");
  }
}