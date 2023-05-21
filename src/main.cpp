#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <StreamString.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESP8266HTTPClient.h>

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



HTTPClient http;
//weather api
//https://randomnerdtutorials.com/esp8266-weather-forecaster/
WiFiClient client;
void makehttpRequest();
void parseJson(const char * jsonString);
const char server1[] = "api.openweathermap.org";
String serverPath="/data/2.5/forecast?id=776175&appid=3db715609a637e8a4ba3f94c421dc703&units=metric";
String text;
int jsonend = 0;
boolean startJson = false;
int status = WL_IDLE_STATUS;
#define JSON_BUFF_DIMENSION 2500
unsigned long lastConnectionTime = 0;     // last time you connected to the server, in milliseconds
const unsigned long postInterval = 10 * 1000;  // posting interval of 10 minutes  (10L * 1000L; 10 seconds delay for testing)


 


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
<p id=\"default\">1</p>\
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


  client.connect(server1, 80);
  http.begin(client, "api.openweathermap.org/data/2.5/forecast?id=776175&appid=3db715609a637e8a4ba3f94c421dc703&units=metric");
  http.GET();

// Print the response
Serial.print(http.getString());


  if (MDNS.begin("esp8266")) { Serial.println("MDNS responder started"); }

  server.on("/", handleRoot);
  server.on("/on", [](){
        digitalWrite(LED_BUILTIN, LOW);                                      //zapal diodę
        digitalWrite(13, HIGH);   // pin13
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
 // Serial.println("przed http requestem");


   

  if (millis() - lastConnectionTime > postInterval) {
    Serial.println("10 sekund");
    // note the time that the connection was made:
    lastConnectionTime = millis();
    makehttpRequest();
  }
 //  Serial.println("po http requestem");
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

// to request data from OWM
void makehttpRequest() {
  // close any connection before send a new request to allow client make connection to server
  client.stop();

  // if there's a successful connection:
  if (client.connect(server1, 80)) {

     Serial.println("connecting...");
    // send the HTTP PUT request:

    client.println("GET " + serverPath);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: Mozilla/5.0");
    //client.println("Connection: close");
    client.println();

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    
    char c = 0;
    while (client.available()) {
      //Serial.println("odczytuje json");
      c = client.read();
      // since json contains equal number of open and close curly brackets, this means we can determine when a json is completely received  by counting
      // the open and close occurences,
      //Serial.print(c);
      if (c == '{') {
        startJson = true;         // set startJson true to indicate json message has started
        jsonend++;
      }
      if (c == '}') {
        jsonend--;
      }
      if (startJson == true) {
        text += c;
      }
      // if jsonend = 0 then we have have received equal number of curly braces 
      if (jsonend == 0 && startJson == true) {
        Serial.println("json odczytany");
        parseJson(text.c_str());  // parse c string text in parseJson function


        text = "";                // clear text string for the next time
        startJson = false;        // set startJson to false to indicate that a new message has not yet started
      }
    }
  }
  else {
    // if no connction was made:
    Serial.println("connection failed");
    return;
  }
}

//to parse json data recieved from OWM
void parseJson(const char * jsonString) {
  Serial.println("parsowanie");
  //StaticJsonBuffer<4000> jsonBuffer;
  const size_t bufferSize = 2*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + 4*JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(2) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 2*JSON_OBJECT_SIZE(7) + 2*JSON_OBJECT_SIZE(8) + 720;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  // FIND FIELDS IN JSON TREE
  JsonObject& root = jsonBuffer.parseObject(jsonString);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  JsonArray& list = root["list"];
  JsonObject& nowT = list[0];


  // including temperature and humidity for those who may wish to hack it in
  
  String city = root["city"]["name"];
  
  float tempNow = nowT["main"]["temp"];
  float humidityNow = nowT["main"]["humidity"];
  String weatherNow = nowT["weather"][0]["description"];


  // checking for four main weather possibilities
  Serial.println("cokolwiek wypisuję");
  Serial.println();
  Serial.print("tempNow: ");
  Serial.println(tempNow);

  Serial.print("humidityNow: ");
  Serial.println(humidityNow);
  
  Serial.println();
}
