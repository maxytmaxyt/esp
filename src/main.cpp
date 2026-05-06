#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

// Define your custom MAC address here
// The first byte should be even (e.g., 0x02, 0x1A, 0x32)
uint8_t customMac[] = {0x02, 0xAD, 0xBE, 0xEF, 0x01, 0x02};

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialize WiFi in Station Mode first
    WiFi.mode(WIFI_STA);

    // esp_wifi_set_mac must be called before WiFi.begin()
    // WIFI_IF_STA selects the station interface
    esp_err_t result = esp_wifi_set_mac(WIFI_IF_STA, &customMac[0]);

    Serial.println("");
    if (result == ESP_OK) {
        Serial.println("MAC address change: SUCCESS");
    } else {
        Serial.println("MAC address change: FAILED");
    }

    // Verify the change
    Serial.print("Active MAC Address: ");
    Serial.println(WiFi.macAddress());

    // Connect to WiFi (uncomment and fill in your credentials)
    /*
    WiFi.begin("Your_SSID", "Your_Password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");
    */
}

void loop() {
    // Empty loop
}
