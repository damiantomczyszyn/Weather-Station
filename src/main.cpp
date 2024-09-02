/*********
  I was inspired by https://randomnerdtutorials.com  
*********/
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <DHT.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include "ESP8266WebServer.h"


const char* ssid     = "";
const char* password = "";

ESP8266WebServer server;
String getWeather();


#define DHTTYPE DHT22 
#define DHTPIN D3
#define WATHER2 D6
#define SEALEVELPRESSURE_HPA (1013.25)


// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;
int val = 0 ;
bool watherSensor = 0;
unsigned long delayTime;
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BME280 bme; 




void handleNotFound() 
{
   server.send(404, "text/plain", "Not found");
}
 

void setup() {
  Serial.begin(9600);
  Serial.println(F("BME280 test"));
  dht.begin();       
  bool status;
  
    status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  pinMode(WATHER2, INPUT_PULLUP);

  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();



// wifi part:
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

server.on("/",[](){server.send(200,"text/plain",getWeather());});
server.on("/json",[](){server.send(200,"text/plain",getJsonWeather());});


ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  
  server.begin();
}
  


void loop() { 
  //printValues();
  //delay(delayTime);
  server.handleClient();
  ArduinoOTA.handle();
  

}



void printValues() {
 
Serial.println(getWeather());  
Serial.println("**************************************************");
Serial.println();
}


String getWeather(){
  String weatherS="";
//WatherSensor1
  val=analogRead(A0); // Water Level Sensor output pin connected A0
  weatherS+="Water lewelIN = ";  // See the Value In Serial Monitor
  weatherS+=val;  // See the Value In Serial Monitor
  weatherS+="\n";

//Humidity DHT
h = dht.readHumidity();
  weatherS+="Humidity = ";
if (isnan(h)) {
   weatherS+="err";
}
else {
   weatherS+=h;
}

//temperature DHT
t = dht.readTemperature();
weatherS+=" Temperature = ";
if (isnan(t)) {
   weatherS+="err";
}
else {
  weatherS+=t;
}
//WatherSensor2 - Main
  
  weatherS+= "\nWather sensor = ";
  if(digitalRead(WATHER2)){
    weatherS+="1";
  }
  else{
   weatherS+="0";
  }  
  weatherS+= "\n";


// BME
  weatherS+="Temperature = ";
  weatherS+=bme.readTemperature();
  weatherS+="*C ";
    
  weatherS+="Pressure = ";
  weatherS+=bme.readPressure() / 100.0F;
  weatherS+="hPa ";

  weatherS+="Approx, Altitude = ";
  weatherS+=bme.readAltitude(SEALEVELPRESSURE_HPA);
  weatherS+="m ";

  weatherS+="Humidity = ";
  weatherS+=bme.readHumidity();
  weatherS+="%";

  weatherS+="\n";


  
return weatherS;
}

String getJsonWeather(){
  String weatherS = "{";

//const char* jsonString = R"({"Temperature": 25.26,"Pressure": 1000.50,"Altitude": 106.70,"Humidity": 28.79})";

  weatherS+="\"Temperature\": ";
  weatherS+=bme.readTemperature();
  weatherS+=",";
    
  weatherS+="\"Pressure\": ";
  weatherS+=bme.readPressure() / 100.0F;
  weatherS+=",";

  weatherS+="\"Altitude\": ";
  weatherS+=bme.readAltitude(SEALEVELPRESSURE_HPA);
  weatherS+=",";

  weatherS+="\"Humidity\": ";
  weatherS+=bme.readHumidity();
  weatherS+=",";

  val=analogRead(A0); // Water Level Sensor output pin connected A0
  weatherS+="\"WaterLewel\": ";  // See the Value In Serial Monitor
  weatherS+=val;  // See the Value In Serial Monitor
  weatherS+=",";


  weatherS+="\"DhtHumidity\": ";
  h = dht.readHumidity();  
  if (isnan(h)) {
    weatherS+="err";
  }
  else {
    weatherS+=h;
  }
  weatherS+=",";

  t = dht.readTemperature();
  weatherS+="\"DhtTemperature\": ";  // See the Value In Serial Monitor
  if (isnan(t)) {
    weatherS+="err";
  }
  else {
    weatherS+=t;
  }
  weatherS+=",";

  weatherS+="\"WatherSensor\": ";  // See the Value In Serial Monitor
  if(digitalRead(WATHER2)){
    weatherS+="1";
  }
  else{
   weatherS+="0";
  }  
  weatherS+=",";

  
  weatherS+= "\Wather sensorIN = ";
  weatherS+= watherSensor;

  weatherS+="}";

  return weatherS;
}
