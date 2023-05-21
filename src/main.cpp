#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <StreamString.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

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

//weather api
//https://randomnerdtutorials.com/esp8266-weather-forecaster/
const char serverAPI[] = "api.openweathermap.org";
String nameOfCity = "STYRZYNIEC,48"; 
String apiKey = "REPLACE_WITH_YOUR_API_KEY"; 
String text;
int jsonend = 0;
boolean startJson = false;
int status = WL_IDLE_STATUS;
#define JSON_BUFF_DIMENSION 2500


unsigned long lastConnectionTime = 10 * 60 * 1000;     // last time you connected to the server, in milliseconds
const unsigned long postInterval = 10 * 60 * 1000;  // posting interval of 10 minutes  (10L * 1000L; 10 seconds delay for testing)

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
  <!--  <meta http-equiv=\"refresh\" content=\"1\"> -->  <!-- przeladowywanie strony co x sekund --> \
</head>\
<body>\
<p id=\"pomiar\">Wartość:</p>\
<!-- zawor nr 1 -->\
<p id=\"default1\">1</p>\
<button id=\"on1\">Włącz</button>\
<button id=\"off1\">Wyłącz</button><br>\
<!-- zawor nr 2 -->\
<p id=\"default2\">2</p>\
<button id=\"on2\">Włącz</button>\
<button id=\"off2\">Wyłącz</button><br>\
<!-- zawor nr 3 -->\
<p id=\"default3\">3</p>\
<button id=\"on3\">Włącz</button>\
<button id=\"off3\">Wyłącz</button><br>\
<!-- zawor nr 4 -->\
<p id=\"default4\">4</p>\
<button id=\"on4\">Włącz</button>\
<button id=\"off4\">Wyłącz</button><br>\
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
    document.getElementById(\"on1\").onclick = function () {const zapytanie = new XMLHttpRequest();zapytanie.open(\"GET\", \"/on1\");zapytanie.send();};\
    document.getElementById(\"off1\").onclick = function () {const zapytanie = new XMLHttpRequest();zapytanie.open(\"GET\", \"/off1\");zapytanie.send();};\
    document.getElementById(\"on2\").onclick = function () {const zapytanie = new XMLHttpRequest();zapytanie.open(\"GET\", \"/on2\");zapytanie.send();};\
    document.getElementById(\"off2\").onclick = function () {const zapytanie = new XMLHttpRequest();zapytanie.open(\"GET\", \"/off2\");zapytanie.send();};\
    document.getElementById(\"on3\").onclick = function () {const zapytanie = new XMLHttpRequest();zapytanie.open(\"GET\", \"/on3\");zapytanie.send();};\
    document.getElementById(\"off3\").onclick = function () {const zapytanie = new XMLHttpRequest();zapytanie.open(\"GET\", \"/off3\");zapytanie.send();};\
    document.getElementById(\"on4\").onclick = function () {const zapytanie = new XMLHttpRequest();zapytanie.open(\"GET\", \"/on4\");zapytanie.send();};\
    document.getElementById(\"off4\").onclick = function () {const zapytanie = new XMLHttpRequest();zapytanie.open(\"GET\", \"/off4\");zapytanie.send();};\
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


  pinMode(ZAW1, OUTPUT);
  pinMode(ZAW2, OUTPUT);
  pinMode(ZAW3, OUTPUT);
  pinMode(ZAW4, OUTPUT);
 

  digitalWrite(LED_BUILTIN, 0);
  Serial.begin(9600);
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

  server.on("/on1", [](){
        digitalWrite(LED_BUILTIN, LOW);                                      //zapal diodę
        digitalWrite(ZAW1, HIGH);   
        server.send(200);
        Serial.println("Dzialanie na zaworze1 - on"); 
           });  
  server.on("/off1", []() {
      digitalWrite(LED_BUILTIN, HIGH);                                        //zgaś diodę
      digitalWrite(ZAW1, LOW); 
      server.send(200);   
      Serial.println("Dzialanie na zaworze1 - off");
  });
  server.on("/on2", [](){
        digitalWrite(LED_BUILTIN, LOW);                                      //zapal diodę
        digitalWrite(ZAW2, HIGH);   
        server.send(200);
        Serial.println("Dzialanie na zaworze2 - on"); 
           });  
  server.on("/off2", []() {
      digitalWrite(LED_BUILTIN, HIGH);                                        //zgaś diodę
      digitalWrite(ZAW2, LOW); 
      server.send(200);   
      Serial.println("Dzialanie na zaworze2 - off");
  });
  server.on("/on3", [](){
        digitalWrite(LED_BUILTIN, LOW);                                      //zapal diodę
        digitalWrite(ZAW3, HIGH);   
        server.send(200);
        Serial.println("Dzialanie na zaworze3 - on"); 
          });  
  server.on("/off3", []() {
      digitalWrite(LED_BUILTIN, HIGH);                                        //zgaś diodę
      digitalWrite(ZAW3, LOW); 
      server.send(200);
      Serial.println("Dzialanie na zaworze3 - off");   
  });
  server.on("/on4", [](){
        digitalWrite(LED_BUILTIN, LOW);                                      //zapal diodę
        digitalWrite(ZAW4, HIGH);   
        server.send(200); 
        Serial.println("Dzialanie na zaworze4 - on");
          });         
  server.on("/off4", []() {
      digitalWrite(LED_BUILTIN, HIGH);                                        //zgaś diodę
      digitalWrite(ZAW4, LOW); 
      server.send(200);   
      Serial.println("Dzialanie na zaworze4 - off");
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