// es_otp_door_lock.ino
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

// -------- CONFIG --------
const char* WIFI_SSID = "somnath";  
const char* WIFI_PASS = "098765431"; 
String BASE_URL = "http://10.243.62.199:8000";

// -------- Keypad pins --------
const int rowPins[4] = {14, 27, 26, 25};
const int colPins[4] = {33, 32, 18, 19};
const char keysMap[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// -------- LCD --------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------- Servo --------
Servo doorServo;
const int servoPin = 23;
const int lockPos = 90;
const int unlockPos = 0;

// -------- Buzzer --------
const int buzzerPin = 13; // Add buzzer pin

// ---------- Buzzer Functions ----------
void beepSuccess() {
  tone(buzzerPin, 1000, 200); // High pitch beep for success
  delay(300);
  noTone(buzzerPin);
}

void beepError() {
  tone(buzzerPin, 300, 500); // Low pitch beep for error
  delay(600);
  noTone(buzzerPin);
}

void beepKeyPress() {
  tone(buzzerPin, 800, 50); // Short beep for key press
  delay(100);
  noTone(buzzerPin);
}

void beepStartup() {
  tone(buzzerPin, 1200, 100);
  delay(150);
  tone(buzzerPin, 1500, 100);
  delay(150);
  tone(buzzerPin, 1800, 100);
  delay(150);
  noTone(buzzerPin);
}

void beepLockUnlock() {
  for(int i = 0; i < 3; i++) {
    tone(buzzerPin, 1000, 100);
    delay(150);
    noTone(buzzerPin);
    delay(50);
  }
}

// ---------- Network Diagnostics ----------
void printNetworkInfo() {
  Serial.println("\n📊 Network Information:");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Laptop IP: ");
  Serial.println("10.243.62.199");
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  
  lcd.clear();
  lcd.print("My IP:");
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());
  delay(3000);
}

// ---------- HTTP helpers ----------
bool httpPostJson(const String &url, const String &jsonBody, String &responseBody, int &httpCode) {
  HTTPClient http;
  
  Serial.print("🔗 Connecting to: ");
  Serial.println(url);
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(10000);
  
  httpCode = http.POST(jsonBody);
  
  if (httpCode > 0) {
    Serial.print("✅ HTTP Response: ");
    Serial.println(httpCode);
    responseBody = http.getString();
    Serial.print("📨 Response: ");
    Serial.println(responseBody);
  } else {
    Serial.print("❌ HTTP Error: ");
    Serial.print(httpCode);
    Serial.print(" - ");
    Serial.println(http.errorToString(httpCode));
    responseBody = "";
  }
  
  http.end();
  return (httpCode > 0);
}

// ---------- Test server connection ----------
void testServerConnection() {
  String url = BASE_URL;  // Test root endpoint
  
  lcd.clear();
  lcd.print("Testing Server...");
  Serial.println("\n🔄 Testing server connection...");
  
  HTTPClient http;
  http.begin(url);
  http.setTimeout(10000);
  
  int httpCode = http.GET();  // Use GET for root endpoint
  
  if (httpCode > 0) {
    Serial.print("✅ HTTP Response: ");
    Serial.println(httpCode);
    String response = http.getString();
    
    if (httpCode == 200) {
      Serial.println("✅ Server connection: SUCCESS");
      lcd.clear();
      lcd.print("Server: OK ✓");
      beepSuccess(); // Buzzer feedback for success
      delay(2000);
      return;
    } else {
      Serial.print("📨 Response: ");
      Serial.println(response);
    }
  } else {
    Serial.print("❌ HTTP Error: ");
    Serial.print(httpCode);
    Serial.print(" - ");
    Serial.println(http.errorToString(httpCode));
  }
  
  // If we get here, connection failed
  Serial.println("❌ Server connection: FAILED");
  lcd.clear();
  lcd.print("Server: FAIL ✗");
  lcd.setCursor(0,1);
  lcd.print("Check Connection");
  beepError(); // Buzzer feedback for error
  delay(3000);
}

// ---------- OTP generation ----------
String generateOTP() {
  String otp = "";
  for (int i = 0; i < 4; ++i) otp += String(random(0, 10));
  return otp;
}

// ---------- Send OTP to backend ----------
bool sendOtpToBackend(const String &otp, const String &mode) {
  String url = BASE_URL + "/send-otp";
  String body = "{\"otp\":\"" + otp + "\",\"mode\":\"" + mode + "\"}";
  String resp; int code;
  bool ok = httpPostJson(url, body, resp, code);
  return ok && (code == 200);
}

// ---------- Verify OTP with backend ----------
bool verifyOtpWithBackend(const String &otp, String &returnedMode) {
  String url = BASE_URL + "/verify-otp";
  String body = "{\"otp\":\"" + otp + "\"}";
  String resp; int code;
  bool ok = httpPostJson(url, body, resp, code);
  
  if (!ok || code != 200) return false;

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, resp);
  if (err) return false;
  
  if (doc.containsKey("status") && doc.containsKey("mode")) {
    String status = doc["status"];
    if (status == "success") {
      returnedMode = doc["mode"].as<String>();
      return true;
    }
  }
  return false;
}

// ---------- Keypad scanning ----------
int scanKeypad() {
  for (int r = 0; r < 4; ++r) {
    for (int i = 0; i < 4; ++i) digitalWrite(rowPins[i], (i == r) ? LOW : HIGH);
    delayMicroseconds(60);
    for (int c = 0; c < 4; ++c) {
      if (digitalRead(colPins[c]) == LOW) {
        delay(20);
        if (digitalRead(colPins[c]) == LOW) return r*4 + c;
      }
    }
  }
  return -1;
}

bool isAnyKeyPressed() {
  for (int r = 0; r < 4; ++r) {
    for (int i = 0; i < 4; ++i) digitalWrite(rowPins[i], (i == r) ? LOW : HIGH);
    for (int c = 0; c < 4; ++c) if (digitalRead(colPins[c]) == LOW) return true;
  }
  return false;
}

// ---------- Read OTP typed on keypad ----------
String readOTPFromKeypad() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter OTP:");
  lcd.setCursor(0,1);
  String input = "";
  while (input.length() < 4) {
    int idx = scanKeypad();
    if (idx >= 0) {
      char ch = keysMap[idx/4][idx%4];
      if (ch >= '0' && ch <= '9') {
        input += ch;
        lcd.print("*");
        beepKeyPress(); // Beep for each key press
      }
      while (isAnyKeyPressed()) delay(10);
      delay(120);
    }
  }
  return input;
}

// ---------- handle lock/unlock flow ----------
void handleLockUnlock(const String &mode) {
  String otp = generateOTP();
  Serial.println("🔐 Generated OTP: " + otp);

  lcd.clear();
  lcd.print("Sending OTP...");
  bool sent = sendOtpToBackend(otp, mode);
  
  if (!sent) {
    lcd.clear();
    lcd.print("Send Failed!");
    lcd.setCursor(0,1);
    lcd.print("Check Server");
    beepError(); // Error beep
    delay(2000);
    return;
  }
  
  lcd.clear();
  lcd.print("OTP Sent!");
  lcd.setCursor(0,1);
  lcd.print("Check Email");
  beepSuccess(); // Success beep for OTP sent
  delay(2000);

  String typed = readOTPFromKeypad();
  String returnedMode = "";
  bool verified = verifyOtpWithBackend(typed, returnedMode);

  if (verified) {
    lcd.clear();
    lcd.print("Verified! ✓");
    beepSuccess(); // Success beep for verification
    delay(1000);
    if (returnedMode == "lock") {
      doorServo.write(lockPos);
      lcd.clear();
      lcd.print("Door Locked 🔒");
      beepLockUnlock(); // Special beep for lock action
    } else {
      doorServo.write(unlockPos);
      lcd.clear();
      lcd.print("Door Unlocked 🔓");
      beepLockUnlock(); // Special beep for unlock action
    }
  } else {
    lcd.clear();
    lcd.print("Invalid OTP!");
    lcd.setCursor(0,1);
    lcd.print("Try Again");
    beepError(); // Error beep for invalid OTP
  }
  delay(2000);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n🚀 Smart Door Lock Starting...");
  randomSeed(analogRead(0));

  // Keypad init
  for (int i = 0; i < 4; ++i) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);
    pinMode(colPins[i], INPUT_PULLUP);
  }

  // Buzzer init
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // LCD init
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Starting...");
  
  // Startup beep
  beepStartup();

  lcd.clear();
  lcd.print("Connecting WiFi");

  // WiFi connection
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("📡 Connecting to WiFi");
  
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
    delay(300);
    Serial.print(".");
    lcd.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected!");
    beepSuccess(); // Success beep for WiFi
    printNetworkInfo();
  } else {
    Serial.println("\n❌ WiFi Failed!");
    lcd.clear();
    lcd.print("WiFi Failed!");
    beepError(); // Error beep for WiFi failure
    while(1) delay(1000);
  }

  // Test server connection
  testServerConnection();

  // Servo setup
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  doorServo.setPeriodHertz(50);
  doorServo.attach(servoPin, 500, 2400);
  doorServo.write(unlockPos);

  lcd.clear();
  lcd.print("Ready!");
  lcd.setCursor(0,1);
  lcd.print("Press # or *");
  
  beepSuccess(); // Final ready beep
}

void loop() {
  int idx = scanKeypad();
  if (idx >= 0) {
    char k = keysMap[idx/4][idx%4];
    while (isAnyKeyPressed()) delay(5);

    if (k == '#') {
      beepKeyPress(); // Beep for key press
      handleLockUnlock("lock");
    } else if (k == '*') {
      beepKeyPress(); // Beep for key press
      handleLockUnlock("unlock");
    }
    
    lcd.clear();
    lcd.print("Ready!");
    lcd.setCursor(0,1);
    lcd.print("Press # or *");
    delay(150);
  }
}
