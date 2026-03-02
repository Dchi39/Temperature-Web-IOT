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

/* ================= GLOBAL DATA ================= */
#define sensor1_read 4
#define sensor2_read 5
#define sensor3_read 6
#define sensor4_read 7

#define coil1_out 12
#define coil2_out 13
#define coil3_out 14
#define coil4_out 15

int coil1 = 150;
int coil2 = 130;
int coil3 = 110;
int coil4 = 90;

String Mode = "heatoff";   // heaton | heatoff | ideal
const size_t jsonSize = 1024;

/* ================= FILE HANDLERS ================= */
void handleRoot() {
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    server.send(404, "text/plain", "index.html not found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void handleStyleCSS() {
  File file = SPIFFS.open("/style.css", "r");
  server.streamFile(file, "text/css");
  file.close();
}

void handleBootstrapCSS() {
  File file = SPIFFS.open("/bootstrap.css", "r");
  server.streamFile(file, "text/css");
  file.close();
}

void handleScriptJS() {
  File file = SPIFFS.open("/script.js", "r");
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleBootstrapJS() {
  File file = SPIFFS.open("/bootstrap.js", "r");
  server.streamFile(file, "application/javascript");
  file.close();
}

/* ================= CONFIG SAVE ================= */
void saveConfig() {
  StaticJsonDocument<jsonSize> doc;

  doc["coil1"] = coil1;
  doc["coil2"] = coil2;
  doc["coil3"] = coil3;
  doc["coil4"] = coil4;
  doc["mode"]  = Mode;

  File file = SPIFFS.open("/config.json", "w");
  if (!file) return;

  serializeJson(doc, file);
  file.close();
}

/* ================= CONFIG LOAD ================= */
void loadConfig() {
  if (!SPIFFS.exists("/config.json")) return;

  File file = SPIFFS.open("/config.json", "r");
  if (!file) return;

  StaticJsonDocument<jsonSize> doc;
  if (deserializeJson(doc, file)) return;

  coil1 = doc["coil1"];
  coil2 = doc["coil2"];
  coil3 = doc["coil3"];
  coil4 = doc["coil4"];
  Mode  = doc["mode"].as<String>();

  file.close();
}

/* ================= SLIDER + MODE ================= */
void handleSlider() {

  if (server.hasArg("coil1")) {
    coil1 = server.arg("coil1").toInt();
    coil2 = server.arg("coil2").toInt();
    coil3 = server.arg("coil3").toInt();
    coil4 = server.arg("coil4").toInt();
  }

  if (server.hasArg("Mmode")) {
    Mode = server.arg("Mmode");
  }

  saveConfig();
  server.send(200, "text/plain", "OK");
}

/* ================= SEND DATA TO WEB ================= */
void handleGetOutput() {
  StaticJsonDocument<jsonSize> doc;

  doc["c1"]   = coil1;
  doc["c2"]   = coil2;
  doc["c3"]   = coil3;
  doc["c4"]   = coil4;
  doc["mode"] = Mode;

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    return;
  }

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  dnsServer.start(DNS_PORT, "*", local_IP);

  server.on("/", handleRoot);

  server.on("/style.css", handleStyleCSS);
  server.on("/bootstrap.css", handleBootstrapCSS);

  server.on("/script.js", handleScriptJS);
  server.on("/bootstrap.js", handleBootstrapJS);

  server.on("/slider", handleSlider);
  server.on("/getOutput", handleGetOutput);

  server.begin();
  loadConfig();

  Serial.println("ESP32 Web Server Started");
  Serial.println(WiFi.softAPIP());
}

/* ================= LOOP ================= */
void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (Mode == "heaton") {
    Serial.print("MODE: HEAT ON");
    Serial.print(" | Coil 1: "); Serial.print(coil1);
    Serial.print(" | Coil 2: "); Serial.print(coil2);
    Serial.print(" | Coil 3: "); Serial.print(coil3);
    Serial.print(" | Coil 4: "); Serial.println(coil4);
  }
  else if (Mode == "heatoff") {
    Serial.println("MODE: HEAT OFF");
  }
  else if (Mode == "ideal") {
    Serial.println("MODE: IDEAL");
  }
}