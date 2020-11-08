#ifndef SMARTVIT_LORA_H
#define SMARTVIT_LORA_H

/* ******************** INCLUDES ******************** */

// Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ******************** DEFINES ******************** */
// Wi-Fi defines
//	Change to the name and password of the network that you are using
#define ESP_SSID "SSID"
#define PASSWORD "PASSWORD"
#define URL_ADDRESS "https://smartvit-data-capture-dev.herokuapp.com/measurement/"    // host + url
#define HOST "https://smartvit-data-capture-dev.herokuapp.com/"
#define METHOD "/measurement/"
#define PORT 8000

//LoRa pins
// Unused pins (commented)
// #define LORA_SCK 5
// #define LORA_MISO 19
// #define LORA_MOSI 27

#define LORA_SS 18
#define LORA_RST 14
#define LORA_DIO0 26

// Available bands
//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 915E6

// OLED pins
// Unused pins (commented)
// #define OLED_SDA        4
// #define OLED_SCL       15 

#define OLED_RST       16
#define SCREEN_WIDTH  128 // OLED display width, in pixels
#define SCREEN_HEIGHT  64 // OLED display height, in pixels

// pins to make connection by software serial with MSP430
#define MSP430_TX 2
#define MSP430_RX 3

// General Defines
#define SERIAL_BAUD_RATE 115200
#define SEALEVELPRESSURE_HPA 1013.25
#define LOOP_TIME     60000 // 60 sec.

// Sensors
#define VREF  1.25
#define RESOLUTION 1023
#define PI 3.1415
#define RADIUS 0.147
#define PERIOD 60
#define SAMPLES 6
#define PLUV_RES 0.25

/* ******************** TYPES AND STRUCTS ******************** */

typedef enum {
  NORTH,      // 0
  NORTHEAST,
  EAST,
  SOUTHEAST,
  SOUTH,
  SOUTHWEST,
  WEST,
  NORTHWEST
}dv10_windsock_t;

typedef struct {
  int     temp_celsius;   // 0
  float   humid_percent;
  float   vento_MS;
  float   pres_hpa;
  float   qnt_chuva;
  float   ph_sensor;
  float   altitude;       // Optional
  dv10_windsock_t vento_direcao;
} data_t;

typedef enum {
  SV10_ANEMOMETER, // 0
  CV10_WINDSOCK,
  PLUVIOMETER_SENSOR,
  BME280_SENSOR,
  PH_SENSOR,
  TEMP_SENSOR, 
  MOIST_SENSOR,
  BME280_TEMP_SENSOR, 
  BME280_PRES_SENSOR,  
  BME280_HUMID_SENSOR,
  BME280_ALTIT_SENSOR,  
  MOIST_SENSOR_1,
  MOIST_SENSOR_2,
  MOIST_SENSOR_3,
}sensor_id_t;


// Generic sensor
// struct sensor_data{ 
//     char sensor_id[20];
//     float value;
//     char date_time[50];
// };

// Sensors structs

struct sensor_anemometro{
    sensor_id_t sensor_id;
    float vento_MS;
};

struct sensor_biruta{
    sensor_id_t sensor_id;
    dv10_windsock_t vento_direcao;
};

struct sensor_pluviometro{
    sensor_id_t sensor_id;
    float qtd_chuva;
};

struct sensor_bme{
    sensor_id_t sensor_id;
    int temp_celsius;
    int humidity_percent;
    float pressure_hPa;
};

struct sensor_ph{
    sensor_id_t sensor_id;
    float sensor_ph;
};

struct sensor_temperatura{
    sensor_id_t sensor_id;
    float temp_soil;
};

struct moist_percent{
  sensor_id_t sensor_id;
  float moist_percent_1;
  float moist_percent_2;
  float moist_percent_3;
};

struct all_sensors_data{
  struct sensor_anemometro sensor_anemometro;
  struct sensor_biruta sensor_biruta;
  struct sensor_pluviometro sensor_pluviometro;
  struct sensor_bme sensor_bme;
  struct sensor_ph sensor_ph;
  struct sensor_temperatura sensor_temperatura;
  struct moist_percent moist_percent;
};

#endif // SMARTVIT_LORA_H
