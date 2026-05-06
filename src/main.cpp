#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <lwip/napt.h>

// ================= SETTINGS START =================
// 1. MAC Address Settings
uint8_t customMac[] = {0x02, 0xAD, 0xBE, 0xEF, 0x01, 0x02};

// 2. Station Settings (The WiFi you connect TO)
const char* ST_SSID = "DEIN_ROUTER_NAME";
const char* ST_PASS = "DEIN_WLAN_PASSWORT";

// 3. SoftAP Settings (The NEW WiFi the ESP creates)
const char* AP_SSID = "ESP32_REPEATER";
const char* AP_PASS = "12345678";
// =================  SETTINGS END  =================

void setup() {
    Serial.begin(115200);
    delay(1000);

    // 1. Set mode to both Station and Access Point
    WiFi.mode(WIFI_AP_STA);

    // 2. Overwrite MAC Address for the Station interface
    // This happens on every boot before connecting
    esp_wifi_set_mac(WIFI_IF_STA, &customMac[0]);

    Serial.println("\n--- ESP32 Startup ---");
    Serial.print("Assigned MAC: ");
    Serial.println(WiFi.macAddress());

    // 3. Start the Access Point (The "Repeated" WiFi)
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.print("AP SSID: ");
    Serial.println(AP_SSID);

    // 4. Connect to your home router
    Serial.print("Connecting to: ");
    Serial.println(ST_SSID);
    WiFi.begin(ST_SSID, ST_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected to home router!");
    Serial.print("IP on home network: ");
    Serial.println(WiFi.localIP());

    // 5. Enable NAPT (Network Address Translation)
    // This is the "magic" that makes it a repeater/router
    // It allows clients on AP_SSID to use the connection from ST_SSID
    ip_napt_enable(WiFi.softAPIP(), 1); 
    
    Serial.println("NAT/Repeater mode active.");
}

void loop() {
    // The ESP32 handles the routing in the background (via LWIP stack)
    // No code needed here for basic repeating functionality
}
