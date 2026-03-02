#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

// ================= WIFI =================
const char* ssid = "heatseal";
const char* password = "12345678";
IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);
DNSServer dnsServer;

// ================= PINS ================= (not used in simulation)
#define coil1_out 12
#define coil2_out 13
#define coil3_out 14
#define coil4_out 15

// ================= CONTROL VARIABLES =================
int setTemp[4] = {150,130,110,90}; // Slider setpoints
double temp[4]; // simulated temperature values
String Mode = "heatoff";
bool safeLock = false;

// ================= WEB HANDLERS =================
void handleRoot() {
    File f = SPIFFS.open("/index.html");
    server.streamFile(f,"text/html");
    f.close();
}

void handleFile(const char* path,const char* type){
    File f = SPIFFS.open(path);
    server.streamFile(f,type);
    f.close();
}

void handleSlider(){
    if(server.hasArg("coil1")){
        setTemp[0] = server.arg("coil1").toInt();
        setTemp[1] = server.arg("coil2").toInt();
        setTemp[2] = server.arg("coil3").toInt();
        setTemp[3] = server.arg("coil4").toInt();
    }
    if(server.hasArg("Mmode")){
        String m = server.arg("Mmode");
        if(m=="ideal"){
            setTemp[0] = setTemp[1] = setTemp[2] = setTemp[3] = 100;
        }
        Mode = m;
    }
    server.send(200,"text/plain","OK");
}

void handleGetOutput(){
    StaticJsonDocument<256> doc;
    doc["c1"] = setTemp[0];
    doc["c2"] = setTemp[1];
    doc["c3"] = setTemp[2];
    doc["c4"] = setTemp[3];
    doc["mode"] = Mode;
    String json;
    serializeJson(doc,json);
    server.send(200,"application/json",json);
}

void handleGetTemperatures(){
    StaticJsonDocument<256> doc;
    doc["t1"] = (int)temp[0];
    doc["t2"] = (int)temp[1];
    doc["t3"] = (int)temp[2];
    doc["t4"] = (int)temp[3];
    String json;
    serializeJson(doc,json);
    server.send(200,"application/json",json);
}

// ================= SETUP =================
void setup(){
    Serial.begin(115200);
    SPIFFS.begin(true);

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_IP,gateway,subnet);
    WiFi.softAP(ssid,password);
    dnsServer.start(53,"*",local_IP);

    // Routes
    server.on("/",handleRoot);
    server.on("/style.css",[](){ handleFile("/style.css","text/css"); });
    server.on("/bootstrap.css",[](){ handleFile("/bootstrap.css","text/css"); });
    server.on("/script.js",[](){ handleFile("/script.js","application/javascript"); });
    server.on("/bootstrap.js",[](){ handleFile("/bootstrap.js","application/javascript"); });
    server.on("/slider",handleSlider);
    server.on("/getOutput",handleGetOutput);
    server.on("/getTemperatures",handleGetTemperatures);
    server.begin();

    randomSeed(analogRead(0)); // seed for random numbers
}

// ================= LOOP =================
void loop(){
    dnsServer.processNextRequest();
    server.handleClient();

    // Simulate temperature changes
    for(int i=0;i<4;i++){
        int change = random(-2,3); // small random change -2 to +2
        temp[i] += change;

        // Clamp to reasonable 0-200
        if(temp[i] < 0) temp[i] = 0;
        if(temp[i] > 200) temp[i] = 200;

        // If in ideal mode, force 100
        if(Mode=="ideal") temp[i] = 100;
    }

    // Simulate safety lock behavior
    if(temp[0] >= setTemp[0]) safeLock = true;
    else if(temp[0] <= setTemp[0]-10) safeLock = false;

    delay(500);
}