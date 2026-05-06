#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

// Improved include for NAPT
#if CONFIG_LWIP_IPV4_NAPT
#include <lwip/napt.h>
#endif

// ================= SETTINGS START =================
uint8_t customMac[] = {0x02, 0xAD, 0xBE, 0xEF, 0x01, 0x02};
const char* ST_SSID = "DEIN_ROUTER_NAME";
const char* ST_PASS = "DEIN_WLAN_PASSWORT";
const char* AP_SSID = "ESP32_REPEATER";
const char* AP_PASS = "12345678";
// =================  SETTINGS END  =================

void setup() {
    Serial.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_AP_STA);

    // Overwrite Station MAC
    esp_wifi_set_mac(WIFI_IF_STA, &customMac[0]);

    Serial.println("\n--- ESP32 Startup ---");
    Serial.print("Assigned MAC: ");
    Serial.println(WiFi.macAddress());

    // Start AP
    WiFi.softAP(AP_SSID, AP_PASS);
    
    // Connect to Router
    WiFi.begin(ST_SSID, ST_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected!");

    // Enable NAT
    // Note: The parameters are (addr, enable)
    // We use the AP Gateway address to start NAT
    #if CONFIG_LWIP_IPV4_NAPT
    ip_napt_enable(WiFi.softAPIP(), 1);
    Serial.println("NAT Enabled");
    #else
    Serial.println("NAPT not supported in this SDK configuration");
    #endif
}

void loop() {}
