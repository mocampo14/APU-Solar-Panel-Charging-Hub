#include <ESP8266WiFi.h>        //Import the wifi library
#include <ESP8266WiFiMulti.h>   //Import the wifi-multi library      
#include <SimpleDHT.h>          //Import SimpleDHT library
#include <ThingSpeak.h>         //Import ThingSpeak library
ESP8266WiFiMulti wifiMulti;     //Create an instance of the ESPwifimulti class called wifiMulti
WiFiClient client;              //Create an instance called client

SimpleDHT11 dht11;                        //Set dht11 as constant to use for SimpleDHT11 library
const int led = BUILTIN_LED;              //Set led as constant for BUILTIN_LED
const int pinDHT11 = D3;                  //Connect DHT to this pin
long lastUpdate = 0;                      //Set constant for lastUpdate to 0 -- used for delay

const int sensor = A0;                    //Set constant for A0 pin
int lightLevel;                     //Constant to read light levels
int battVolt;                       //Constant to read battery voltage
int ampLevel;                       //Constant to ready amps level
int solarVolt;                      //Constant to read solar voltage

//Setup MUX pins here:
const int pin1 = 12;
const int pin2 = 13;
const int pin3 = 14; 

//Setup for ThingSpeak here:
unsigned long channelNumber = ;               //ThingSpeak channel number -- int here
const char* apiKey = "";                      // Thingspeak API Key -- string here
const char* host = "api.thingspeak.com";      // API host server 

////////////////////////////////
// Method to connect to wifi ///
////////////////////////////////
void connectWiFi() {
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());         //Tells us the MACAdrress of the module
  while (wifiMulti.run() != WL_CONNECTED) {  //Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250);
    Serial.print('.');
    digitalWrite(led, !digitalRead(led));
  }
  digitalWrite(led, LOW);

  Serial.println('\n');
  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());               //Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());            //Tells us the IP address that we're connected to
}

////////////////////////////////////////////////////
/// Method read values in A0 pin connected to MUX // 
////////////////////////////////////////////////////
void readValue() {
  //LOW == OFF
  //HIGH == ON
  
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);
  delay(1000);
  lightLevel = analogRead(sensor);
  ThingSpeak.setField(3, lightLevel);

  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, HIGH);
  delay(1000);
  battVolt = analogRead(sensor);
  ThingSpeak.setField(4, battVolt);

  digitalWrite(pin1, LOW);
  digitalWrite(pin2, HIGH);
  digitalWrite(pin3, LOW);
  delay(1000);
  ampLevel = analogRead(sensor);
  ThingSpeak.setField(5, ampLevel);

  digitalWrite(pin1, HIGH);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);
  delay(1000);
  solarVolt = analogRead(sensor);
  ThingSpeak.setField(6, solarVolt);
}

////////////////////////////////
// Method to push the data   ///
////////////////////////////////
void pushData(){
  // start working...
  Serial.println("=================================");
  Serial.println("APU Solar Charging Hub...");
  
  // read without samples
  byte temperature = 0;
  byte humidity = 0;

  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(pinDHT11, &temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); Serial.println(err);delay(2000);
    return;
  }

  readValue();   //Run the readValue method
  
  Serial.print("Reading OK: ");
  Serial.print((int)temperature); Serial.print(" *C, "); 
  Serial.print((int)humidity); Serial.print(" H, ");
  Serial.print((int)lightLevel); Serial.print(" L, ");
  Serial.print((int)battVolt); Serial.print(" BV, ");
  Serial.print((int)ampLevel); Serial.print(" A, ");
  Serial.print((int)solarVolt); Serial.println(" SV ");

  //Write the data to ThingSpeak
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.writeFields(channelNumber, apiKey);
}

void setup() {
  Serial.begin(115200);
  delay(10);

  //add wifi connections here:
  wifiMulti.addAP("");                  //Connect to this network
  
  ThingSpeak.begin(client);             //Start ThingSpeak client
  connectWiFi();                        //Run connectWifi method

  //Set up so the pins can be used to output
  //These are interchangeable
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);

  //Set up so the pins are off in the beginning
  //These are interchangeable
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);
}

void loop() {
  long currentTime = millis();
  if (currentTime - lastUpdate >= 20000) {
    lastUpdate = currentTime;     //update value for lastUpdate
    pushData();                   //Run pushdata method when if statement is met
  }
}
