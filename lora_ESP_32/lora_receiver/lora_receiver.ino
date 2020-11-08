/* ******************** INCLUDES ******************** */
// SmartVit Libraries (SV)
#include "SmartVit_lora.h"

//Libraries for Server
#include <ESP8266WiFi.h> 
#include <ESP8266HTTPClient.h> 

// LoRa library
#include <SPI.h>
#include <LoRa.h>

// Additional Libraries
#include <ArduinoJson.h>

/* ******************** GLOBAL DATA******************** */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// Variables used to general purposes
// struct to storage data received from the LoRa sender. 
// struct sensor_data data;
struct all_sensors_data total_data;
DynamicJsonBuffer jsonBuffer;

int value = 0; 

/* ******************** Settings to send data to server ******************** */
// Define request attributes
// Variables used to connect to the server
const char* ssid = ESP_SSID; 
const char* password = PASSWORD; 
const char* host = HOST; //edit the host adress, ip address etc. 

String url = METHOD; 


/* ******************** FUNCTIONS ******************** */

// SETUP AND LOOP

void setup() { 
  //initialize Serial Monitor
  Serial.begin(SERIAL_BAUD_RATE);
  init_wifi();
  init_oled();
  init_lora();

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA RECEIVER ");
  display.display();
}

void loop() {

  //try to parse packet
  String str_data;
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    //received a packet
    Serial.print("Received packet ");

    //read packet
    while (LoRa.available()) {
      str_data = (String)LoRa.read();
      Serial.print(str_data);
    }
    
    JsonObject& root = jsonBuffer.parseObject(str_data);

    //print RSSI of packet
    int rssi = LoRa.packetRssi();
    Serial.print(" with RSSI ");    
    Serial.println(rssi);

   // Display information
   display.clearDisplay();
   display.setCursor(0,0);
   display.print("LORA RECEIVER");
   display.setCursor(0,20);
   display.print("Received packet:");
   display.setCursor(0,30);
   display.print(str_data);
   display.setCursor(0,40);
   display.print("RSSI:");
   display.setCursor(30,40);
   display.print(rssi);
   display.display(); 

   // send to server
  String output;
  root.printTo(output);
 
   sendToServer(output);
  }
}


void init_lora(){
  //SPI LoRa pins
  SPI.begin();
  
  //setup LoRa transceiver module
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,10);
  display.println("LoRa Initializing OK!");
  display.display();  
}

// ADDIOTIONAL FUNCTIONS
void init_oled(){
  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);
  
  //initialize OLED
  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
}


void init_wifi()
{
  Serial.begin(SERIAL_BAUD_RATE); 
  delay(10); // We start by connecting to a WiFi network 
  Serial.println(); 
  Serial.println(); Serial.print("Connecting to "); 
  Serial.println(ssid); 
  
  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default, would try to act as both a client and an access-point and could cause network-issues with your other WiFi-devices on your WiFi-network. */ 
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  } 
  
  Serial.println(""); 
  Serial.println("WiFi connected"); 
  Serial.println("IP address: "); 
}

void sendToServer(String dataToSend){

 if(WiFi.status() == WL_CONNECTED){   //Check WiFi connection status
 
   HTTPClient http;    //Declare object of class HTTPClient
 
   http.begin(URL_ADDRESS);      //Specify request destination
   http.addHeader("Content-Type", "application/json");  //Specify content-type header
 
   int httpCode = http.POST(dataToSend);   //Send the request
   String payload = http.getString();                  //Get the response payload
 
   Serial.println(httpCode);   //Print HTTP return code
   Serial.println(payload);    //Print request response payload
 
   http.end();  //Close connection
 
 }else{
 
    Serial.println("Error in WiFi connection");   
 
 }
}
