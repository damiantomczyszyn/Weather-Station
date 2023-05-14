#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <StreamString.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <DHT.h>
#define DHTPIN 12   
#define DHTTYPE DHT22   // DHT 22  (AM2302)

#ifndef STASSID
#define STASSID "PLAY_internet_2.4G_6D7" 
#define STAPSK "7GpfT5CG"// TODO ustawić jako zmienna srodowiskowa aby haslo bylo ukryte
#endif

#define SIGNAL_PIN A0 //wather sensor data pin

#define ZAW1 D0 //zawory
#define ZAW2 D1 //zawory
#define ZAW3 D2 //zawory
#define ZAW4 D3 //zawory

int value = 0; // variable to store the sensor wather value

DHT dht(DHTPIN, DHTTYPE);

const char *ssid = STASSID;
const char *password = STAPSK;

//declare functions
String getTime();
String dateAndTime=" ";
//

ESP8266WebServer server(80);


void handleRoot() {

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  value = analogRead(SIGNAL_PIN); // read the analog value from sensor


  StreamString temp;
  temp.reserve(500);  // Preallocate a large chunk to avoid memory fragmentation
  temp.printf("\
<html>\
<head>\
    <title>Strona</title>\
    <meta charset=\"UTF-8\"/>\
    <meta http-equiv=\"refresh\" content=\"1\">\
</head>\
<body>\
<p id=\"pomiar\">Wartość:</p>\
<button id=\"on\">Włącz</button>\
<button id=\"off\">Wyłącz</button><br>\
<button id=\"download\">Feature for later</button>");
temp.print("<p>Sensor Value =  ");
  if (isnan(t) || isnan(h)) {
    temp.println("Failed to read from DHT");
  } else {
    temp.print("Humidity: "); 
    temp.print(h);
    temp.print(" %\t");
    temp.print("Temperature: "); 
    temp.print(t);
    temp.println(" *C");
  }
temp.println("</p>");
temp.println("Time:");
temp.println(getTime());

temp.print("<p>Wather sensor Value =  ");

  temp.println(value);

temp.println("</p>");

temp.printf("\
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
  dht.begin();


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

bool wathering(){
  return false;
}

String getTime(){
const long utcOffsetInSeconds = 3600;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);


  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
  }
  timeClient.begin();
  timeClient.update();

  Serial.print(daysOfTheWeek[timeClient.getDay()]);
  Serial.print(", ");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());
  //Serial.println(timeClient.getFormattedTime());
  dateAndTime=daysOfTheWeek[timeClient.getDay()]+timeClient.getHours()+timeClient.getMinutes()+timeClient.getSeconds();

  return (String)timeClient.getFormattedTime();

}

int difTime(){
  return 0;
}