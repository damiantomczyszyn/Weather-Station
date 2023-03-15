#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <StreamString.h>

#ifndef STASSID
#define STASSID "PLAY_internet_2.4G_6D7"
#define STAPSK "7GpfT5CG"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);


void handleRoot() {
  StreamString temp;
  temp.reserve(500);  // Preallocate a large chunk to avoid memory fragmentation
  temp.printf("\
<html>\
<head>\
    <title>Strona</title>\
    <meta charset=\"UTF-8\"/>\
</head>\
<body>\
<p id=\"pomiar\">Wartość:</p>\
<button id=\"on\">Włącz</button>\
<button id=\"off\">Wyłącz</button><br>\
<button id=\"download\">Pobierz obrazek</button>\
<script>\
    document.getElementById(\"on\").onclick = function () {const zapytanie = new XMLHttpRequest();zapytanie.open(\"GET\", \"/on\");zapytanie.send();};\
    document.getElementById(\"off\").onclick = function () {const zapytanie = new XMLHttpRequest();zapytanie.open(\"GET\", \"/off\");zapytanie.send();};\
</script>\
</body>\
</html>" );
  server.send(200, "text/html", temp.c_str());

}

void handleNotFound() {
  digitalWrite(LED_BUILTIN, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }

  server.send(404, "text/plain", message);
  digitalWrite(LED_BUILTIN, 0);
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) { Serial.println("MDNS responder started"); }

  server.on("/", handleRoot);
  server.on("/on", [](){
        digitalWrite(LED_BUILTIN, LOW);                                      //zapal diodę
        digitalWrite(13, HIGH);   
        server.send(200);   });
  
  server.on("/off", []() {
      digitalWrite(LED_BUILTIN, HIGH);                                        //zgaś diodę
      digitalWrite(13, LOW); 
      server.send(200);   
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}