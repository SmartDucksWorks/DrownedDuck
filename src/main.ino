/**************************************************************
 * SmartDucks ESP-01S Firmware
 * Deep Sleep + OTA (MD5 + Safe Mode) — Optimized Build
 * ------------------------------------------------------------
 * Version: v1.0.0
 * Target:  ESP8266 ESP-01S (1 MB flash)
 * Author:  SmartDucks Team
 **************************************************************/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

// ======== CONFIGURATION ========
const char* WIFI_SSID  = "RebelScum";
const char* WIFI_PASS  = "FengHuangR2D2#ShenLung";
const char* VERSION_URL = "https://github.com/SmartDucksWorks/DrownedDuck/releases/latest/download/version.json";
const char* CURRENT_VERSION = "v1.0.0";

const uint8_t EEPROM_SIZE = 64;
const uint8_t ADDR_VER  = 0;
const uint8_t ADDR_SAFE = 60;
const uint8_t SAFE_MAGIC = 0xA5;

const unsigned long DEEP_SLEEP_S = 300; // seconds
// ===============================


// ======== UTILITY FUNCTIONS ========
void eepromWriteStr(uint8_t addr, const String& s) {
  EEPROM.begin(EEPROM_SIZE);
  for (uint8_t i = 0; i < s.length() && i < 31; i++) EEPROM.write(addr + i, s[i]);
  EEPROM.write(addr + s.length(), 0);
  EEPROM.commit(); EEPROM.end();
}

String eepromReadStr(uint8_t addr) {
  EEPROM.begin(EEPROM_SIZE);
  char buf[32];
  for (uint8_t i = 0; i < 31; i++) buf[i] = EEPROM.read(addr + i);
  buf[31] = 0; EEPROM.end();
  return String(buf);
}

bool safeMode() {
  EEPROM.begin(EEPROM_SIZE);
  bool s = EEPROM.read(ADDR_SAFE) == SAFE_MAGIC;
  EEPROM.end();
  return s;
}
void setSafeMode(bool on) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(ADDR_SAFE, on ? SAFE_MAGIC : 0);
  EEPROM.commit(); EEPROM.end();
}
// ==================================


// ======== WIFI + OTA =========
bool connectWiFi(uint16_t timeout = 10000) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < timeout) delay(250);
  return WiFi.status() == WL_CONNECTED;
}

bool doOTA(bool verbose = true) {
  WiFiClientSecure c; c.setInsecure();
  HTTPClient h;
  if (!h.begin(c, VERSION_URL)) return false;
  if (h.GET() != HTTP_CODE_OK) { h.end(); return false; }

  StaticJsonDocument<384> d;
  if (deserializeJson(d, h.getString())) { h.end(); return false; }
  h.end();

  String latest = d["version"], url = d["url"], md5 = d["md5"];
  if (verbose) Serial.printf("Ver %s → %s\n", CURRENT_VERSION, latest.c_str());
  if (latest == CURRENT_VERSION) return true; // Up-to-date

  if (verbose) Serial.println("🚀 Updating...");
  eepromWriteStr(ADDR_VER, latest);

  t_httpUpdate_return r = ESPhttpUpdate.update(c, url, CURRENT_VERSION, md5);
  return (r == HTTP_UPDATE_OK);
}

void goSleep() {
  Serial.printf("😴 Sleeping %lus...\n", DEEP_SLEEP_S);
  ESP.deepSleep(DEEP_SLEEP_S * 1e6);
}
// ==============================


// ======== MAIN LOGIC ========
void setup() {
  Serial.begin(115200);
  delay(150);
  Serial.println("\n🦆 SmartDucks Boot");

  if (safeMode()) {
    Serial.println("⚠️ Safe Mode OTA recovery...");
    if (connectWiFi() && doOTA(false)) setSafeMode(false);
    goSleep();
  }

  if (!connectWiFi()) {
    Serial.println("❌ WiFi failed → flag safe mode");
    setSafeMode(true);
    goSleep();
  }

  if (!doOTA()) {
    Serial.println("❌ OTA failed → flag safe mode");
    setSafeMode(true);
  } else setSafeMode(false);


  // === SMARTDUCKS LOGIC START =====================================
  // 
  //  Insert your SmartDucks operational logic here.
  //  Examples:
  //    - Initialize sensors (e.g. DHT, analog, Dallas, etc.)
  //    - Collect environmental or system data
  //    - Transmit to server or local hub (HTTP/MQTT)
  //    - Perform any decision logic
  // 
  //  Keep runtime short — device will deep sleep after this block.
  // 
  // ---------------------------------------------------------------
  
  Serial.println("🐤 Running SmartDucks core logic...");

  // Example placeholder:
  // float temp = readTemperature();
  // sendData(temp);
  // delay(100);  // allow transmission

  // === SMARTDUCKS LOGIC END =======================================


  goSleep();
}

void loop() {} // Not used; all logic runs in setup()
