#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>

// ---------------- WiFi & Firebase Credentials ----------------
#define WIFI_SSID "Airtel_Mighty Raju"
#define WIFI_PASSWORD "Mitaoe#701"

#define API_KEY "AIzaSyAUxLwgUALpPcWpp7_5GwHyNHsgNJQO3O8"
#define DATABASE_URL "https://smart-agriculture-df146-default-rtdb.firebaseio.com/"

// ---------------- Sensor Pins ----------------
#define DHTPIN D4
#define DHTTYPE DHT11
#define ANALOG_PIN A0     // pH sensor or Soil Moisture sensor

DHT dht(DHTPIN, DHTTYPE);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(500);

  // WiFi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n✅ WiFi connected!");

  // Firebase setup
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("✅ Firebase sign-up successful!");
  } else {
    Serial.printf("❌ Sign-up Error: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  dht.begin();
  Serial.println("✅ Firebase initialized!");
}

// ---------------- Read pH Sensor ----------------
float readPH() {
  int samples = 10;
  float total = 0;

  for (int i = 0; i < samples; i++) {
    total += analogRead(ANALOG_PIN);
    delay(10);
  }

  float avgADC = total / samples;
  float voltage = avgADC * (3.3 / 1023.0);

  // Correct pH formula for typical pH probe circuit
  float phValue = 7 + ((1.65 - voltage) / 0.18);

  return phValue;
}

// ---------------- Read Soil Moisture Sensor ----------------
int readMoisture() {
  int val = analogRead(ANALOG_PIN);
  return map(val, 1023, 0, 0, 100);
}

// ---------------- Main Loop ----------------
void loop() {

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int moisture = readMoisture();
  float pH = readPH();

  Serial.println("\n📊 Sensor Readings:");
  Serial.printf("Temperature: %.2f °C\n", temperature);
  Serial.printf("Humidity: %.2f %%\n", humidity);
  Serial.printf("Moisture: %d %%\n", moisture);
  Serial.printf("pH Value: %.2f\n", pH);

  if (Firebase.ready()) {
    String path = "/SmartFarm/Readings/" + String(millis());

    Firebase.RTDB.setFloat(&fbdo, path + "/Temperature", temperature);
    Firebase.RTDB.setFloat(&fbdo, path + "/Humidity", humidity);
    Firebase.RTDB.setInt(&fbdo, path + "/Moisture", moisture);
    Firebase.RTDB.setFloat(&fbdo, path + "/pH", pH);

    Serial.println("✅ Data uploaded to Firebase!");
  }

  delay(5000);
}
