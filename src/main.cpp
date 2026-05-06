#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

// IP_NAPT is set via build_flags (-D IP_NAPT=1), so we use that macro.
// CONFIG_LWIP_IPV4_NAPT is a Kconfig/sdkconfig macro — NOT set by build_flags!
#if IP_NAPT
#include <lwip/napt.h>
#include <lwip/dns.h>
#endif

// ================= SETTINGS START =================
uint8_t     customMac[]  = { 0x02, 0xAD, 0xBE, 0xEF, 0x01, 0x02 };
const char* ST_SSID      = "DEIN_ROUTER_NAME";
const char* ST_PASS      = "DEIN_WLAN_PASSWORT";
const char* AP_SSID      = "ESP32_REPEATER";
const char* AP_PASS      = "12345678";

// AP address block — must be different from the upstream router's subnet
const IPAddress AP_IP   (192, 168, 4, 1);
const IPAddress AP_GW   (192, 168, 4, 1);
const IPAddress AP_MASK (255, 255, 255, 0);

// How long to wait for STA connection before giving up (ms)
const uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
// =================  SETTINGS END  =================

// ---------------------------------------------------------------------------
// Connects to the upstream router. Returns true on success.
// ---------------------------------------------------------------------------
static bool connectToRouter() {
    Serial.printf("[STA] Connecting to \"%s\" ...\n", ST_SSID);
    WiFi.begin(ST_SSID, ST_PASS);

    const uint32_t deadline = millis() + WIFI_CONNECT_TIMEOUT_MS;
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() > deadline) {
            Serial.println("\n[STA] Timeout — connection failed.");
            WiFi.disconnect(false);
            return false;
        }
        delay(500);
        Serial.print(".");
    }

    Serial.printf("\n[STA] Connected! IP: %s  GW: %s\n",
        WiFi.localIP().toString().c_str(),
        WiFi.gatewayIP().toString().c_str());
    return true;
}

// ---------------------------------------------------------------------------
// Enables NAT on the AP interface.
// ip_napt_enable() expects the AP's IP as a u32_t (host-byte-order on ESP32).
// Casting IPAddress to uint32_t is safe: IPAddress::operator uint32_t()
// returns the internal 32-bit value that lwIP understands.
// ---------------------------------------------------------------------------
static void enableNAT() {
#if IP_NAPT
    const uint32_t apIP = (uint32_t)AP_IP;   // explicit, unambiguous cast
    err_t err = ip_napt_enable(apIP, 1);
    if (err == ERR_OK) {
        Serial.println("[NAT] Enabled successfully.");
    } else {
        Serial.printf("[NAT] ip_napt_enable() failed, err=%d\n", (int)err);
    }

    // Forward the upstream DNS server to clients on the AP side
    const ip_addr_t* dns = dns_getserver(0);
    if (dns && !ip_addr_isany(dns)) {
        dns_setserver(0, dns);   // keeps the DNS pointer in sync after NAPT
        Serial.printf("[DNS] Forwarding DNS: %s\n",
            IPAddress(dns->u_addr.ip4.addr).toString().c_str());
    }
#else
    Serial.println("[NAT] NAPT not compiled in — add -D IP_NAPT=1 to build_flags.");
#endif
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n===== ESP32 WiFi Repeater =====");

    // Mode must be set before touching the WiFi driver
    WiFi.mode(WIFI_AP_STA);

    // --- Custom Station MAC --------------------------------------------------
    esp_err_t macErr = esp_wifi_set_mac(WIFI_IF_STA, customMac);
    if (macErr == ESP_OK) {
        Serial.printf("[MAC] Station MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
            customMac[0], customMac[1], customMac[2],
            customMac[3], customMac[4], customMac[5]);
    } else {
        Serial.printf("[MAC] Warning: esp_wifi_set_mac() failed (0x%x). "
                      "Custom MAC not applied.\n", macErr);
        // Non-fatal: we continue with the burned-in MAC
    }

    // --- Access Point --------------------------------------------------------
    // softAPConfig() MUST be called before softAP() so the DHCP server
    // hands out addresses from the correct subnet from the very first client.
    WiFi.softAPConfig(AP_IP, AP_GW, AP_MASK);

    if (WiFi.softAP(AP_SSID, AP_PASS)) {
        Serial.printf("[AP]  Started: SSID=\"%s\"  IP=%s\n",
            AP_SSID, WiFi.softAPIP().toString().c_str());
    } else {
        Serial.println("[AP]  ERROR: softAP() failed!");
        // Continuing is pointless without an AP; reboot after a pause
        delay(5000);
        ESP.restart();
    }

    // --- Station (upstream) --------------------------------------------------
    if (!connectToRouter()) {
        Serial.println("[STA] Will keep retrying in loop().");
    }

    // --- NAT -----------------------------------------------------------------
    // NAT is activated on the AP netif; works as long as the AP is up,
    // even if the upstream link hasn't come up yet.
    enableNAT();

    Serial.println("===== Setup complete =====\n");
}

// ---------------------------------------------------------------------------
// Loop — handles upstream reconnection
// ---------------------------------------------------------------------------
void loop() {
    static uint32_t lastReconnectAttempt = 0;

    if (WiFi.status() != WL_CONNECTED) {
        const uint32_t now = millis();
        // Attempt reconnect at most once every 10 seconds
        if (now - lastReconnectAttempt >= 10000) {
            lastReconnectAttempt = now;
            Serial.println("[STA] Lost upstream connection — reconnecting...");
            WiFi.disconnect(false);
            if (!connectToRouter()) {
                Serial.println("[STA] Reconnect failed, will try again...");
            }
        }
    }

    delay(1000);
}
