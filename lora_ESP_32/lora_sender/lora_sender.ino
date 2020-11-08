/* ******************** INCLUDES ******************** */
// SmartVit Libraries (SV)
#include "SmartVit_lora.h"

// LoRa library
#include <SPI.h>
#include <LoRa.h>

// BME280 Libraries
#include <SoftwareSerial.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

/* ******************** GLOBAL DATA******************** */
// Packet counter
int counter = 0;
unsigned long loop_init_time = 0;

float calibration = 0.00;
float wind_dir = 0.00;

Adafruit_BME280 bme; // I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
SoftwareSerial MSP430(MSP430_RX, MSP430_TX); // RX, TX

// Variables used to general purposes
// struct to storage data that will be sent to the LoRa receiver. 
struct all_sensors_data total_data;

/* ******************** FUNCTIONS ******************** */


// Additional Functions

void lora_init(){
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
  display.print("LoRa Initializing OK!");
  display.display();
  delay(2000);
}

void oled_init(){
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


void get_data(all_sensors_data *total_data){
  // First read the sensor identifier to let us know what is being sent 
  sensor_id_t data_id = (sensor_id_t) MSP430.read();
  
  while(MSP430.available()){
    switch(data_id){
      case PH_SENSOR:
        total_data->sensor_ph.sensor_id = PH_SENSOR;
        total_data->sensor_ph.sensor_ph = MSP430.parseFloat(); 
        total_data->sensor_ph.sensor_ph = (total_data->sensor_ph.sensor_ph * VREF) / (RESOLUTION * SAMPLES); 
        total_data->sensor_ph.sensor_ph = -5.70 * total_data->sensor_ph.sensor_ph + calibration ;
        break;

      case TEMP_SENSOR:
        total_data->sensor_temperatura.sensor_id = TEMP_SENSOR;
        total_data->sensor_temperatura.temp_soil = MSP430.parseFloat(); 
        total_data->sensor_temperatura.temp_soil = (total_data->sensor_temperatura.temp_soil * VREF) / RESOLUTION; 
        break;
      
      case SV10_ANEMOMETER:      
        total_data->sensor_biruta.sensor_id = SV10_ANEMOMETER;
        total_data->sensor_anemometro.vento_MS = MSP430.parseFloat();
        total_data->sensor_anemometro.vento_MS = (2 * PI * RADIUS * total_data->sensor_anemometro.vento_MS)/PERIOD;
        break;
      
      case CV10_WINDSOCK:
        total_data->sensor_biruta.sensor_id = CV10_WINDSOCK;
        wind_dir = (dv10_windsock_t)MSP430.read();
        wind_dir = (wind_dir * VREF) / RESOLUTION;
        if (wind_dir > 3.00){
          total_data->sensor_biruta.vento_direcao  = NORTH;
        }
        else if (wind_dir > 2.80){
          total_data->sensor_biruta.vento_direcao = NORTHEAST;
        }
        else if (wind_dir > 2.70){
          total_data->sensor_biruta.vento_direcao = EAST;
        }
        else if (wind_dir > 2.50){
          total_data->sensor_biruta.vento_direcao = SOUTHEAST;
        }
        else if (wind_dir > 2.30){
          total_data->sensor_biruta.vento_direcao = SOUTH;
        }
        else if (wind_dir > 2.20){
          total_data->sensor_biruta.vento_direcao = SOUTHWEST;
        }
        else if (wind_dir > 2.05){
          total_data->sensor_biruta.vento_direcao = WEST;
        }
        else if (wind_dir > 1.95){
          total_data->sensor_biruta.vento_direcao = NORTHWEST;
        }
        else{
          total_data->sensor_biruta.vento_direcao = NORTHEAST;
        }
        break;

      case PLUVIOMETER_SENSOR:
        total_data->sensor_pluviometro.sensor_id = PLUVIOMETER_SENSOR;
        total_data->sensor_pluviometro.qtd_chuva= (total_data->sensor_pluviometro.qtd_chuva *PLUV_RES) * PERIOD;
        total_data->sensor_pluviometro.qtd_chuva = MSP430.read();
        break;
      
      case BME280_TEMP_SENSOR:
        // reads temperature in Celsius
        total_data->sensor_bme.sensor_id = BME280_SENSOR;
        total_data->sensor_bme.temp_celsius = bme.readTemperature();
        break;
      
      case BME280_HUMID_SENSOR:
        // reads absolute humidity
        total_data->sensor_bme.sensor_id = BME280_SENSOR;
        total_data->sensor_bme.humidity_percent = bme.readHumidity();
        break;
      
      case BME280_PRES_SENSOR:
        // reads pressure in hPa (hectoPascal = millibar)
        total_data->sensor_bme.sensor_id = BME280_SENSOR;
        total_data->sensor_bme.pressure_hPa = bme.readPressure() / 100.0F;
        break;
      
      case MOIST_SENSOR_1:
        // reads absolute humidity
        total_data->moist_percent.sensor_id = MOIST_SENSOR;
        total_data->moist_percent.moist_percent_1 = MSP430.parseFloat();
        total_data->moist_percent.moist_percent_1 = (total_data->moist_percent.moist_percent_1 * VREF) / RESOLUTION;
        break;

      case MOIST_SENSOR_2:
        // reads absolute humidity
        total_data->moist_percent.sensor_id = MOIST_SENSOR;
        total_data->moist_percent.moist_percent_2 = MSP430.parseFloat();
        total_data->moist_percent.moist_percent_2 = (total_data->moist_percent.moist_percent_2 * VREF) / RESOLUTION;
        break;

      case MOIST_SENSOR_3:
        // reads absolute humidity
        total_data->moist_percent.sensor_id = MOIST_SENSOR;
        total_data->moist_percent.moist_percent_3 = MSP430.parseFloat();
        total_data->moist_percent.moist_percent_3 = (total_data->moist_percent.moist_percent_3 * VREF) / RESOLUTION;
        break;
    }
  }
}

// SETUP AND LOOP

void setup() {
  // Init 
  Serial.begin(SERIAL_BAUD_RATE);
  oled_init();
  lora_init();

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA SENDER ");
  display.display();
}

void loop() {
  loop_init_time = millis();  
  
  Serial.print("Sending packet: ");
  Serial.println(counter);

  get_data(&total_data);
  
  LoRa.idle();
  
  //Send LoRa packet to receiver
  LoRaSendPacket();
  
  LoRa.sleep();
  delay(LOOP_TIME - loop_init_time);
  
  LoRa.begin(BAND);
}


void LoRaSendPacket(){
  LoRa.beginPacket();
    LoRa.print("{\"data\": {");
    LoRa.write((unsigned char *)&total_data.sensor_anemometro.sensor_id, sizeof(sensor_id_t));
    LoRa.print(": {\"vento_MS\": ");
    LoRa.write((unsigned char *)&total_data.sensor_anemometro.vento_MS, sizeof(total_data.sensor_anemometro.vento_MS));
    LoRa.print("}, ");
    LoRa.write((unsigned char *)&total_data.sensor_biruta.sensor_id, sizeof(sensor_id_t));
    LoRa.print(": {\"vento_direcao\": ");
    LoRa.write((unsigned char *)&total_data.sensor_biruta.vento_direcao, sizeof(total_data.sensor_biruta.vento_direcao));
    LoRa.print("}, ");
    LoRa.write((unsigned char *)&total_data.sensor_pluviometro.sensor_id, sizeof(sensor_id_t));
    LoRa.print(": {\"qtd_chuva\": ");
    LoRa.write((unsigned char *)&total_data.sensor_pluviometro.qtd_chuva, sizeof(total_data.sensor_pluviometro.qtd_chuva));
    LoRa.print("}, ");
    LoRa.write((unsigned char *)&total_data.sensor_bme.sensor_id, sizeof(sensor_id_t));
    LoRa.print(": {\"temp_celsius\": ");
    LoRa.write((unsigned char *)&total_data.sensor_bme.temp_celsius, sizeof(total_data.sensor_bme.temp_celsius));
    LoRa.print(", \"humidity_percent\": ");
    LoRa.write((unsigned char *)&total_data.sensor_bme.humidity_percent, sizeof(total_data.sensor_bme.humidity_percent));
    LoRa.print(", \"pressure_hPa\": ");
    LoRa.write((unsigned char *)&total_data.sensor_bme.pressure_hPa, sizeof(total_data.sensor_bme.pressure_hPa));
    LoRa.print("}, ");
    LoRa.write((unsigned char *)&total_data.sensor_ph.sensor_id, sizeof(sensor_id_t));
    LoRa.print(": {\"sensor_ph\": ");
    LoRa.write((unsigned char *)&total_data.sensor_ph.sensor_ph, sizeof(total_data.sensor_ph.sensor_ph));
    LoRa.print("}, ");
    LoRa.write((unsigned char *)&total_data.sensor_temperatura.sensor_id, sizeof(sensor_id_t));
    LoRa.print(": {\"sensor_ph\": ");
    LoRa.write((unsigned char *)&total_data.sensor_temperatura.temp_soil, sizeof(total_data.sensor_temperatura.temp_soil));
    LoRa.print("}, ");
    LoRa.write((unsigned char *)&total_data.moist_percent.sensor_id, sizeof(sensor_id_t));
    LoRa.print(": {\"moist_percent_1\": ");
    LoRa.write((unsigned char *)&total_data.moist_percent.moist_percent_1, sizeof(total_data.moist_percent.moist_percent_1));
    LoRa.print(", \"moist_percent_2\": ");
    LoRa.write((unsigned char *)&total_data.moist_percent.moist_percent_2, sizeof(total_data.moist_percent.moist_percent_2));
    LoRa.print(", \"moist_percent_3\": ");
    LoRa.write((unsigned char *)&total_data.moist_percent.moist_percent_3, sizeof(total_data.moist_percent.moist_percent_3));    
    LoRa.print("}}}");
  LoRa.endPacket();
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("LORA SENDER");
  display.setCursor(0,20);
  display.setTextSize(1);
  display.print("LoRa packet sent.");
  display.setCursor(0,30);    
  display.display();
  
}
