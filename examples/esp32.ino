#include <TinyGPS++.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

// === GPS setup ===
// ESP32 UART2 pinleri (HardwareSerial)
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
HardwareSerial &neogps = Serial2;
TinyGPSPlus gps;
double lat = 0.0, lng = 0.0;

// === Wi-Fi & HTTP setup ===
// Kendi ağ bilgilerinizi girin
const char* ssid       = "S22";
const char* password   = "A1234567";
// POST endpoint
const char* serverURL  = "https://map.arda.engineer/location";
// POST aralığı (ms)
const unsigned long postInterval = 60UL * 1000;
unsigned long lastPostTime = 0;

// --- Yardımcı fonksiyonlar ---
String charToString(char c) {
  char tmp[2] = { c, '\0' };
  return String(tmp);
}

String readLineFromStream(Stream &stream) {
  String line;
  while (stream.available()) {
    char c = stream.read();
    line += charToString(c);
    if (c == '\n') break;
  }
  return line;
}

void printLatLng(double latitude, double longitude) {
  Serial.print("Latitude : ");  Serial.println(latitude, 6);
  Serial.print("Longitude: ");  Serial.println(longitude, 6);
}

void updateLatLngIfMatch(const String &line) {
  if (!line.startsWith("$GPRMC")) return;
  int star = line.indexOf('*');
  String clean = (star > 0 ? line.substring(0, star) : line);

  String parts[12];
  int idx = 0, start = 0;
  for (int i = 0; i < clean.length() && idx < 12; i++) {
    if (clean.charAt(i) == ',') {
      parts[idx++] = clean.substring(start, i);
      start = i + 1;
    }
  }
  if (idx < 7 || parts[2] != "A") return;

  // Enlem ddmm.mmmm, N/S
  double rawLat = parts[3].toDouble();
  char ns = parts[4].charAt(0);
  double dLat = floor(rawLat / 100.0);
  double mLat = rawLat - dLat * 100.0;
  lat = dLat + mLat / 60.0;
  if (ns == 'S') lat = -lat;

  // Boylam dddmm.mmmm, E/W
  double rawLng = parts[5].toDouble();
  char ew = parts[6].charAt(0);
  double dLng = floor(rawLng / 100.0);
  double mLng = rawLng - dLng * 100.0;
  lng = dLng + mLng / 60.0;
  if (ew == 'W') lng = -lng;
}

void sendLocation() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, skipping POST");
    return;
  }
  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");
  String payload = String("{\"lat\":") + String(lat, 6)
                 + ",\"lng\":" + String(lng, 6) + "}";
  int code = http.POST(payload);
  if (code > 0) {
    Serial.printf("POST %s → %d\n", serverURL, code);
    Serial.println(http.getString());
  } else {
    Serial.printf("POST error: %s\n", http.errorToString(code).c_str());
  }
  http.end();
}

void setup() {
  Serial.begin(115200);

  // GPS UART2 başlat (9600, 8N1) RX2=16, TX2=17
  neogps.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  // Wi-Fi bağlantısı
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, password);
  Serial.printf("WiFi \"%s\" bağlanıyor", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi bağlı, IP: " + WiFi.localIP().toString());
}

void loop() {
  // GPS'ten gelen NMEA satırlarını oku ve TinyGPS++'a ver
  while (neogps.available()) {
    String nmea = readLineFromStream(neogps);
    Serial.println(nmea);
    updateLatLngIfMatch(nmea);
    for (char c : nmea) gps.encode(c);
  }

  // TinyGPS++ güncellemesi
  if (gps.location.isUpdated()) {
    lat = gps.location.lat();
    lng = gps.location.lng();
    Serial.println("Koordinat güncellendi!");
  }

  // Seri port çıktısı
  printLatLng(lat, lng);

  // Periyotla POST at
  if (millis() - lastPostTime >= postInterval) {
    sendLocation();
    lastPostTime = millis();
  }

  delay(1000);
}