#define SW_VERSION " ThinkSpeak.com" // SW version will appear at initial LCD Display
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DHTPIN D4    // digital pin connected to DHT sensor
#define DHTTYPE DHT11   // type of DHT sensor (DHT11, DHT22, AM2302, etc.)
#define echoPin D0 // pin connected to Echo of HC-SR04
#define trigPin D3 // pin connected to Trig of HC-SR04
#define buzzerPin D6 // pin connected to the buzzer

DHT dht(DHTPIN, DHTTYPE);

const char* MY_SSID = "vineeth";
const char* MY_PWD = "123456789";
const char* TS_SERVER = "api.thingspeak.com";
const String TS_API_KEY = "XIFYZONASJ6UYO14";

WiFiClient client;

void connectWifi() {
  Serial.print("Connecting to ");
  Serial.println(MY_SSID);
  WiFi.begin(MY_SSID, MY_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
}

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  
  lcd.begin(16,2);
  lcd.backlight();
  lcd.clear();
  lcd.print("WELCOME");
  
  dht.begin();
  
  connectWifi();
}

void loop() {
  long duration;
  int distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Temperature: "));
  Serial.println(t);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("temp:");
  lcd.print(t);
  lcd.setCursor(0,1);
  lcd.print("Distance:");
  lcd.print(distance); 
  
  if (distance > 10) {
    Serial.println("Fault detected");
    digitalWrite(buzzerPin,HIGH); // Activate buzzer
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Distance:");
  lcd.print(distance); 
  lcd.setCursor(0,1);
  lcd.print("fault dtected");
    delay(1000);
    
  } else {
    Serial.println("No fault");
    digitalWrite(buzzerPin,LOW); // Activate buzzer
    delay(1000);
  }

  if (t > 40) {
    Serial.println("High Temperature (chance)");
  digitalWrite(buzzerPin,HIGH);  // Activate buzzer
    
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("temp:");
  lcd.print(t);
  lcd.setCursor(0,1);
  lcd.print("High temp");
    delay(1000); 
  } else {
  digitalWrite(buzzerPin,LOW); 
    delay(1000);
  }

  if (client.connect(TS_SERVER, 80)) {
    String postStr = TS_API_KEY;
    postStr += "&field1=";
    postStr += String(distance);
    postStr += "&field2=";
    postStr += String(t);
    postStr += "&field3=";
    postStr += String(h);
    // Add additional fields if needed
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + TS_API_KEY + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    delay(1000);
  }
}
