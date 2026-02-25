#include<ArduinoJson.h>
#include<WiFi.h>
#include<HTTPClient.h>
#include<vector>

volatile bool needpush=false;
const char* ap_ssid = "ESP-AP";
const char* ap_password = "12345678";
const char* serverUrl = "https://serverapi.net";
const char* sta_ssid = "vivo Y300 5G";
const char* sta_password = "1234567890";

#define PUSH_INTERVAL 5000
const char* IGNORE_MAC="C0:A8:04:04:00:00";

struct clientinfo{
  String mac;
};
std::vector<clientinfo> clients;
unsigned long lastpushtime = 0;

String mactostring(const uint8_t* mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  return String(macStr);
}
void pushtocloud(){
  if(WiFi.status()!=WL_CONNECTED){
    Serial.println("NO INTERNET CONNECTION");
    return;
  }

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  
  JsonDocument doc;
  doc["decive_id"]="ESP-32-01";
  doc["ap_ip"]=WiFi.softAPIP().toString();
  JsonArray arr=doc["clients"].to<JsonArray>();
for(int i=0;i<clients.size();i++){
  JsonObject obj=arr.add<JsonObject>();
  obj["mac"]=clients[i].mac;
}
String payload;
serializeJson(doc,payload);
int httpcode=http.POST(payload);
Serial.println(payload);
Serial.println("Cloud POST status: " + String(httpcode));

http.end();
}

void wifievent(WiFiEvent_t event,WiFiEventInfo_t info){
  if(event==ARDUINO_EVENT_WIFI_AP_STACONNECTED){
    String mac=mactostring(info.wifi_ap_staconnected.mac);
    if(mac==IGNORE_MAC) return;

    clients.push_back({mac});
    Serial.println("Client connected : "+ mac);
    needpush=true;
  }

  if(event==ARDUINO_EVENT_WIFI_AP_STADISCONNECTED){
    String mac = mactostring(info.wifi_ap_stadisconnected.mac);
    if(mac==IGNORE_MAC)return;
      clients.erase(
      std::remove_if(
        clients.begin(),
        clients.end(),
        [&](clientinfo &c){return c.mac==mac;}
      ),
      clients.end()
      );
       Serial.println("Client disconnected: " + mac);
    needpush=true;
  }
}
void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.onEvent(wifievent);

  WiFi.begin(sta_ssid,sta_password);
  Serial.print("Connecting to Station");
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Station: "+ WiFi.SSID());

  WiFi.softAP(ap_ssid,ap_password);
  Serial.println("AP Started with SSID: "+ String(ap_ssid));
}
void loop(){
if(needpush){
  needpush=false;
  pushtocloud();
  lastpushtime=millis();
}
if(millis()-lastpushtime>PUSH_INTERVAL){
  lastpushtime=millis();
  pushtocloud();
}
}