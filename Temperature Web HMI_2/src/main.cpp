#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

/* ================= WIFI ================= */
const char *ssid = "heatseal";
const char *password = "1234";

IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

DNSServer dnsServer;
WebServer server(80);
const byte DNS_PORT = 53;

/* ================= COIL & SENSOR SETUP ================= */
int coilSet[4] = {150, 130, 110, 90};
String Mode = "heatoff";
bool overheatFlag = false;

const size_t jsonSize = 1024;

/* ================= UTIL FUNCTIONS ================= */
void serveFile(const char *path, const char *type) {
    File f = SPIFFS.open(path, "r");
    if (!f) {
        server.send(404, "text/plain", "File not found");
        return;
    }
    server.streamFile(f, type);
    f.close();
}

/* ================= CONFIG ================= */
void saveConfig() {
    StaticJsonDocument<jsonSize> d;
    for (int i = 0; i < 4; i++) d["coil"][i] = coilSet[i];
    d["mode"] = Mode;
    File f = SPIFFS.open("/config.json", "w");
    serializeJson(d, f);
    f.close();
}

void loadConfig() {
    if (!SPIFFS.exists("/config.json")) return;
    File f = SPIFFS.open("/config.json", "r");
    StaticJsonDocument<jsonSize> d;
    if (!deserializeJson(d, f)) {
        for (int i = 0; i < 4; i++) coilSet[i] = d["coil"][i];
        Mode = d["mode"].as<String>();
    }
    f.close();
}

/* ================= WEB HANDLERS ================= */
void handleSlider() {
    if (server.hasArg("coil1")) {
        coilSet[0] = server.arg("coil1").toInt();
        coilSet[1] = server.arg("coil2").toInt();
        coilSet[2] = server.arg("coil3").toInt();
        coilSet[3] = server.arg("coil4").toInt();
    }
    if (server.hasArg("Mmode")) Mode = server.arg("Mmode");

    saveConfig();
    server.send(200, "text/plain", "OK");
}

void handleGetOutput() {
    // ---------------- SIMULATE TEMPERATURES ----------------
    float temp[4];
    for (int i = 0; i < 4; i++) {
        temp[i] = random(50, 150); // 50°C to 160°C
    }

    overheatFlag = false;
    for (int i = 0; i < 4; i++)
        if (temp[i] > coilSet[i] + 60) overheatFlag = true;

    // ---------------- JSON OUTPUT ----------------
    StaticJsonDocument<jsonSize> d;
    d["c1"] = coilSet[0];
    d["c2"] = coilSet[1];
    d["c3"] = coilSet[2];
    d["c4"] = coilSet[3];
    d["mode"] = Mode;

    d["t1"] = temp[0];
    d["t2"] = temp[1];
    d["t3"] = temp[2];
    d["t4"] = temp[3];

    d["overheat"] = overheatFlag;

    String out;
    serializeJson(d, out);
    server.send(200, "application/json", out);
}

/* ================= SETUP ================= */
void setup() {
    Serial.begin(115200);
    SPIFFS.begin(true);

    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    dnsServer.start(DNS_PORT, "*", local_IP);

    server.on("/", []() { serveFile("/index.html", "text/html"); });
    server.on("/live.html", []() { serveFile("/live.html", "text/html"); });
    server.on("/style.css", []() { serveFile("/style.css", "text/css"); });
    server.on("/script.js", []() { serveFile("/script.js", "application/javascript"); });
    server.on("/live.js", []() { serveFile("/live.js", "application/javascript"); });
    server.on("/chart.js", []() { serveFile("/chart.js", "application/javascript"); });
    server.on("/bootstrap.css", []() { serveFile("/bootstrap.css", "text/css"); });
    server.on("/bootstrap.js", []() { serveFile("/bootstrap.js", "application/javascript"); });
    server.on("/slider", handleSlider);
    server.on("/getOutput", handleGetOutput);

    server.begin();
    loadConfig();
    Serial.println("ESP32 Heat Seal Controller Ready");
    Serial.println(WiFi.softAPIP());
}

/* ================= LOOP ================= */
void loop() {
    dnsServer.processNextRequest();
    server.handleClient();
}