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
unsigned long WHEEL_TRIGGER = 0;
unsigned long WHEEL_PREV_TRIGGER = 0;
unsigned long PEDAL_TRIGGER = 0;
unsigned long PEDAL_PREV_TRIGGER = 0;
int new_wheel_trigger=0;
float WHEEL_RPM;
float PEDAL_RPM;
volatile uint32_t PEDAL_PIN_STATE=0;
volatile uint32_t PREV_PEDAL_PIN_STATE=0;


void wheel_hall_trigger(){  
  WHEEL_PREV_TRIGGER = WHEEL_TRIGGER;
  WHEEL_TRIGGER = millis();
  new_wheel_trigger = 1;
}
void pedal_hall_trigger(){  
  PEDAL_PREV_TRIGGER = PEDAL_TRIGGER;
  PEDAL_TRIGGER = millis();
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

// NeoPixel setup
Adafruit_NeoMatrix LED_MATRIX = Adafruit_NeoMatrix(5, 8, LED_PIN,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);  
  
void update_lights(){
  if(millis()-matrix_time>matrix_frametime){
    matrix_index++;
    if(matrix_index>=bitmap_length){matrix_index=0;}
    matrix_time = millis();
    LED_MATRIX.fillScreen(0);
  }
  if(R_BLINK_STATE>0 && L_BLINK_STATE>0){
    LED_MATRIX.drawBitmap(0,0,hazard_blinker_bmp[matrix_index],5,8,LED_RED_HIGH);
    bitmap_length = 5;
  }
  else if(R_BLINK_STATE>0){
    LED_MATRIX.drawBitmap(0,0,right_blinker_bmp[matrix_index],5,8,LED_RED_HIGH);
    bitmap_length = 2;
  }
  else if(L_BLINK_STATE>0){
    LED_MATRIX.drawBitmap(0,0,left_blinker_bmp[matrix_index],5,8,LED_RED_HIGH);
    bitmap_length = 2;
  }
  else{LED_MATRIX.fillScreen(0);matrix_index=0;}
}

// Accelerometer setup
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

// Magnetometer setup
Adafruit_LSM303DLH_Mag_Unified mag = Adafruit_LSM303DLH_Mag_Unified(12345);

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
  sensors_event_t event;
  accel.getEvent(&event);
  mag.getEvent(&event);
  
  PREV_L_PIN_STATE = L_PIN_STATE;
  L_PIN_STATE=digitalRead(L_BLINKER_PIN);
  PREV_R_PIN_STATE = R_PIN_STATE;
  R_PIN_STATE=digitalRead(R_BLINKER_PIN);

  if(PREV_R_PIN_STATE - R_PIN_STATE==1){
    R_BLINK_STATE = -R_BLINK_STATE;
  }
  if(PREV_L_PIN_STATE - L_PIN_STATE==1){
    L_BLINK_STATE = -L_BLINK_STATE;
  }

  update_lights();
  
  if(new_wheel_trigger==1){
    WHEEL_RPM = (1.0/(WHEEL_TRIGGER - WHEEL_PREV_TRIGGER))*60000;  
  }
  
  PREV_PEDAL_PIN_STATE=PEDAL_PIN_STATE;
  PEDAL_PIN_STATE=digitalRead(PEDAL_HE_PIN);
  if(PREV_PEDAL_PIN_STATE-PEDAL_PIN_STATE==1){
    pedal_hall_trigger();
    PEDAL_RPM = (1.0/(PEDAL_TRIGGER - PEDAL_PREV_TRIGGER))*60000;  
  }
  
  Serial.print("HEAD");Serial.print(",");
  printInt(gps.satellites.value(), gps.satellites.isValid(), 5);Serial.print(",");
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);Serial.print(",");
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);Serial.print(",");
  printDateTime(gps.date, gps.time);Serial.print(",");
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);Serial.print(",");
  Serial.print(R_BLINK_STATE);Serial.print(",");
  Serial.print(L_BLINK_STATE);Serial.print(",");
  Serial.print(PEDAL_RPM);Serial.print(","); 
  Serial.print(WHEEL_RPM);Serial.print(","); 
  Serial.print(event.acceleration.x);Serial.print(",");
  Serial.print(event.acceleration.y);Serial.print(",");
  Serial.print(event.acceleration.z);Serial.print(",");
  Serial.print(event.magnetic.x);Serial.print(",");
  Serial.print(event.magnetic.y);Serial.print(",");
  Serial.print(event.magnetic.z);Serial.println(",");
   
  LED_MATRIX.show();
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

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }
  
  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}
