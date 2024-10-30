/***************************************************
  This is an example sketch for our optical Fingerprint sensor
  Designed specifically to work with the Adafruit BMP085 Breakout
  ----> http://www.adafruit.com/products/751
 ****************************************************/

#include <Adafruit_Fingerprint.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
WiFiClient wifiClient;
// WiFi তথ্য
const char* ssid = "TP-Link";
const char* password = "85413419";

// API URL
String apiUrl = "http://192.168.0.107/upload_fingerprint.php";

// সফটওয়্যার সিরিয়াল সেটআপ
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(13, 15);  // RX, TX
#else
#define mySerial Serial1
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(9600);
  while (!Serial); 
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // WiFi সংযোগ
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // ফিঙ্গারপ্রিন্ট সেন্সর আরম্ভ করা
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  } else {
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}

void loop() {
  getFingerprintID();
  delay(1000);  // বারবার চেক করার জন্য অপেক্ষা করুন
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // ইমেজ সফল হলে প্রক্রিয়া চালিয়ে যান
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // ফিঙ্গারপ্রিন্ট অনুসন্ধান করা
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");

    // HTTP POST রিকোয়েস্ট পাঠানো
    if (WiFi.status() == WL_CONNECTED) { 
       HTTPClient http;
    http.begin(wifiClient, apiUrl);  // WiFiClient যুক্ত করুন
    http.addHeader("Content-Type", "application/json");

      // JSON ফরম্যাটে ডেটা পাঠানো
      String postData = "{\"id\": " + String(finger.fingerID) + "}";
      int httpResponseCode = http.POST(postData);

      // রেসপন্স প্রিন্ট করা
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
  } else {
    Serial.println("Unknown error");
  }

  // সফল হলে আইডি ও কনফিডেন্স প্রিন্ট করা
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}
