#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include "led_display.h"

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Adafruit_LSM303_Accel.h>
#include <Adafruit_LSM303DLH_Mag.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

//#define DEBUG

// Pin defintion
#define GPS_RX_PIN 10
#define GPS_TX_PIN 9
#define PEDAL_HE_PIN 8
#define WHEEL_HE_PIN 7 
#define L_BLINKER_PIN 5
#define R_BLINKER_PIN 4
#define LED_PIN 6

// GPS setup
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(GPS_RX_PIN, GPS_TX_PIN);

// Variables to handle hall sensor tachometer
unsigned long WHEEL_TRIGGER_TIME = 0;
unsigned long WHEEL_PREV_TRIGGER_TIME = 0;
unsigned long PEDAL_TRIGGER_TIME = 0;
unsigned long PEDAL_PREV_TRIGGER_TIME = 0;
bool WHEEL_TRIGGER=false;
float PEDAL_TRIGGER_COUNT=0.0;
float WHEEL_RPM;
float PEDAL_RPM;
volatile uint32_t PEDAL_PIN_STATE=0;
volatile uint32_t PREV_PEDAL_PIN_STATE=0;
const int L_PTT = 10;
unsigned long PEDAL_TRIGGER_DELTAS[L_PTT] = {0,0,0,0,0,0,0,0,0,0};
int PTT_ARRAY_INDEX = 0;

void wheel_hall_trigger(){  
  WHEEL_PREV_TRIGGER_TIME = WHEEL_TRIGGER_TIME;
  WHEEL_TRIGGER_TIME = millis();
  WHEEL_RPM = (1.0/(WHEEL_TRIGGER_TIME - WHEEL_PREV_TRIGGER_TIME))*60000;  
}

void pedal_hall_interrupt(){  
  PREV_PEDAL_PIN_STATE=PEDAL_PIN_STATE;
  PEDAL_PIN_STATE=digitalRead(PEDAL_HE_PIN);
  if(PREV_PEDAL_PIN_STATE-PEDAL_PIN_STATE==1){
    increment_pedal_array();
  }
}

void increment_pedal_array(){
    PEDAL_TRIGGER_DELTAS[PTT_ARRAY_INDEX] = millis()-PEDAL_TRIGGER_TIME;
    PEDAL_TRIGGER_TIME = millis();
    PTT_ARRAY_INDEX += 1;
    if(PTT_ARRAY_INDEX >= L_PTT){
      PTT_ARRAY_INDEX = 0;
    }
}


//Variables to handle blinkers
int L_PIN_STATE = 0;
int PREV_L_PIN_STATE = 0;
int R_PIN_STATE = 0;
int PREV_R_PIN_STATE = 0;
int R_BLINK_STATE = -1; //-1 is off, 1 is on.
int L_BLINK_STATE = -1; 
int matrix_index = 0;
unsigned long matrix_time = millis();
int matrix_frametime = 300; //ms, time between frames
int bitmap_length = 0;
int BLINKER_STATE;

// NeoPixel setup
Adafruit_NeoMatrix LED_MATRIX = Adafruit_NeoMatrix(5, 8, LED_PIN,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);  
  
// Accelerometer setup
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

// Magnetometer setup
Adafruit_LSM303DLH_Mag_Unified mag = Adafruit_LSM303DLH_Mag_Unified(12345);

// Serial Messaging Setup
int GPS_YEAR;
int GPS_MONTH;
int GPS_DAY;
int GPS_HOUR; 
int GPS_MIN;
int GPS_SEC;
int GPS_SAT;
float GPS_LAT;
float GPS_LONG;
float GPS_ALT;
float MSG_PEDAL_RPM;
float MSG_WHEEL_RPM;
int MSG_BLINKER_STATE;
unsigned long MILLISECONDS = millis();
byte incomingByte;
bool start_bool = false;
bool lights_bool = false;
bool brake_bool = false;
const float brake_thresh = -5.0;

void update_lights(){
  if(lights_bool){
    if(brake_bool){
        LED_MATRIX.setBrightness(255);
        LED_MATRIX.fillScreen(LED_RED_HIGH);
    }
    else{
        LED_MATRIX.setBrightness(255);
        LED_MATRIX.fillScreen(LED_RED_HIGH);
    }
  }
  else{
    LED_MATRIX.fillScreen(0);
  }
}


void setup()
{ 
  pinMode(PEDAL_HE_PIN,INPUT); // Change pinmode for hall effect sensor digital pins to input.
  pinMode(WHEEL_HE_PIN,INPUT); 
  pinMode(L_BLINKER_PIN,INPUT); // Change pinmode for button digital pins to input.
  pinMode(R_BLINKER_PIN,INPUT);

  attachInterrupt(digitalPinToInterrupt(WHEEL_HE_PIN),wheel_hall_trigger,FALLING);
  
  LED_MATRIX.begin();           // Initialize NeoPixel object 
  LED_MATRIX.show();            // Turn OFF all pixels
  LED_MATRIX.setBrightness(200); // Set brightness to about 1/5 (max = 255)
  
  ss.begin(GPSBaud); // Initialize GPS device

  accel.begin(); // Initialize accelerometer
  mag.begin(); // Initialize magnetometer
  
  Serial.begin(115200);
}

void loop()
{
  while(Serial.available()){
     incomingByte = Serial.read();
     if(incomingByte==126){
      start_bool = true;
     }
     else if(incomingByte==43){
      lights_bool = true;
     }
     else if(incomingByte==45){
      lights_bool = false;
     }
  }
  
  sensors_event_t event;
  accel.getEvent(&event);
  mag.getEvent(&event);
  
  pedal_hall_interrupt();

  if(millis()-WHEEL_TRIGGER_TIME > 2000){ //if no hall trigger in 2s, assume wheel is still.
    WHEEL_RPM = 0;
  }
    
  if(millis()-PEDAL_TRIGGER_TIME>2000){ //if no hall trigger in 2s, move to next trigger.
    increment_pedal_array();
  }
  
  PEDAL_RPM = 0;
  if( !(PEDAL_TRIGGER_DELTAS[PTT_ARRAY_INDEX]>2000 && PEDAL_TRIGGER_DELTAS[PTT_ARRAY_INDEX-1]>2000 && PEDAL_TRIGGER_DELTAS[PTT_ARRAY_INDEX-2]>2000) ){
    for(int j=0; j<L_PTT; j++){
      PEDAL_RPM += 1.0/(PEDAL_TRIGGER_DELTAS[j])*60000;
    }
    PEDAL_RPM = PEDAL_RPM/L_PTT;
    if(PEDAL_RPM<=31){PEDAL_RPM = 0;}
  }
     
  if(gps.date.isValid()){
    if(gps.date.year() != GPS_YEAR || start_bool){
      GPS_YEAR = gps.date.year();
      Serial.print("YEA,");Serial.print(GPS_YEAR);Serial.println(",");
    }
    if(gps.date.month() != GPS_MONTH || start_bool){
      GPS_MONTH = gps.date.month();
      Serial.print("MON,");Serial.print(GPS_MONTH);Serial.println(",");
    }
    if(gps.date.day() != GPS_DAY || start_bool){
      GPS_DAY = gps.date.day();
      Serial.print("DAY,");Serial.print(GPS_DAY);Serial.println(",");
    }
  }
  if(gps.time.isValid()){
    if(gps.time.hour() != GPS_HOUR || start_bool){
      GPS_HOUR = gps.time.hour();
      Serial.print("HOU,");Serial.print(GPS_HOUR);Serial.println(",");
    }
    if(gps.time.minute() != GPS_MIN || start_bool){
      GPS_MIN = gps.time.minute();
      Serial.print("MIN,");Serial.print(GPS_MIN);Serial.println(",");
    }
    if(gps.time.second() != GPS_SEC || start_bool){
      GPS_SEC = gps.time.second();
      Serial.print("SEC,");Serial.print(GPS_SEC);Serial.println(",");
      MILLISECONDS=millis();
    }
  }
  if((gps.satellites.value() != GPS_SAT && gps.satellites.isValid()) || start_bool){
    GPS_SAT = gps.satellites.value();
    Serial.print("SAT,");Serial.print(GPS_SAT);Serial.println(",");
  } 
  if((gps.location.lat() != GPS_LAT && gps.location.isValid()) || start_bool){
    GPS_LAT = gps.location.lat();
    Serial.print("LAT,");Serial.print(GPS_LAT,6);Serial.println(",");
  }     
  if((gps.location.lng() != GPS_LONG && gps.location.isValid()) || start_bool){
    GPS_LONG = gps.location.lng();
    Serial.print("LON,");Serial.print(GPS_LONG,6);Serial.println(",");
  }     
  if((gps.altitude.meters() != GPS_ALT && gps.altitude.isValid()) || start_bool){
    GPS_ALT = gps.altitude.meters();
    Serial.print("ALT,");Serial.print(GPS_ALT,2);Serial.println(",");
  }
  if( (PEDAL_RPM != MSG_PEDAL_RPM || start_bool) && (PEDAL_RPM<150) ){
    MSG_PEDAL_RPM = PEDAL_RPM;
    Serial.print("PED,");Serial.print(MSG_PEDAL_RPM);Serial.println(",");
  }
  if( (WHEEL_RPM != MSG_WHEEL_RPM || start_bool) && (WHEEL_RPM<500) ){
    MSG_WHEEL_RPM = WHEEL_RPM;
    Serial.print("WHE,");Serial.print(MSG_WHEEL_RPM);Serial.println(",");
  }
  if(BLINKER_STATE != MSG_BLINKER_STATE || start_bool){
    MSG_BLINKER_STATE = BLINKER_STATE;
    Serial.print("BLI,");Serial.print(MSG_BLINKER_STATE);Serial.println(",");
  }

  Serial.print("IMU,");Serial.print(millis()-MILLISECONDS);Serial.print(",");
  Serial.print(event.acceleration.x);Serial.print(",");
  Serial.print(event.acceleration.y);Serial.print(",");
  Serial.print(event.acceleration.z);Serial.print(",");
  Serial.print(event.magnetic.x);Serial.print(",");
  Serial.print(event.magnetic.y);Serial.print(",");
  Serial.print(event.magnetic.z);Serial.println(",");
  if(start_bool){
    Serial.println("END,");
  }
  if(event.magnetic.z<brake_thresh){brake_bool=true;}
  else{brake_bool=false;}
  update_lights();
  LED_MATRIX.show();
  
  smartDelay(20);
  start_bool=false;  
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
