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
#include <TimedAction.h>

// TODO
// - Blinker behavior to neopixel, all inside arduino, status messages to raspi
// - Mag + Accel integration, status messages to raspi, Accel integration with neopixel (brake light)
// - Hall Effect sensor integration, functions for determining speed + cadence, status messages to raspi
// - Light sensor integration with neopixel
// - Temp sensor, pressure sensor, humidity sensor?? car proximity indicator??

#define DEBUG

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
volatile uint32_t PEDAL_HE_COUNT;
volatile uint32_t WHEEL_HE_COUNT;
unsigned long WHEEL_TIME = 0;

//Variables to handle blinkers
volatile uint32_t L_PIN_PRESS_COUNT;
volatile uint32_t R_PIN_PRESS_COUNT;
static const int BLINKER_PRESS_COUNT=5;
int R_BLINK_STATE = -1; //-1 is off, 1 is on.
int L_BLINK_STATE = -1; 
int matrix_index = 0;
int matrix_time = millis();
int matrix_frametime = 300; //ms, time between frames
int bitmap_length = 0;

// NeoPixel setup
Adafruit_NeoMatrix LED_MATRIX = Adafruit_NeoMatrix(5, 8, LED_PIN,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);  
  
void blinker(){
  if(matrix_time-millis()>matrix_frametime){
    matrix_index++;
    if(matrix_index>=bitmap_length){matrix_index=0;}
    matrix_time = millis();
    LED_MATRIX.fillScreen(0);
  }
  if(R_BLINK_STATE>0 && L_BLINK_STATE>0){
    LED_MATRIX.drawBitmap(0,0,hazard_blinker_bmp[matrix_index],5,8,LED_RED_HIGH);
    bitmap_length = 6;
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
TimedAction blinkerThread = TimedAction(10,blinker);

// Accelerometer setup
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

// Magnetometer setup
Adafruit_LSM303DLH_Mag_Unified mag = Adafruit_LSM303DLH_Mag_Unified(12345);

void wheel_hall_toggle(){  
  WHEEL_HE_COUNT++;
}

void setup()
{ 
  pinMode(PEDAL_HE_PIN,INPUT); // Change pinmode for hall effect sensor digital pins to input.
  pinMode(WHEEL_HE_PIN,INPUT); 
  pinMode(L_BLINKER_PIN,INPUT); // Change pinmode for button digital pins to input.
  pinMode(R_BLINKER_PIN,INPUT);

  attachInterrupt(digitalPinToInterrupt(WHEEL_HE_PIN),wheel_hall_toggle,FALLING);
  
  LED_MATRIX.begin();           // Initialize NeoPixel object 
  LED_MATRIX.show();            // Turn OFF all pixels
  LED_MATRIX.setBrightness(50); // Set brightness to about 1/5 (max = 255)
  
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

  if(digitalRead(R_BLINKER_PIN)==HIGH){
    R_PIN_PRESS_COUNT++;
  }
  else{R_PIN_PRESS_COUNT=0;}
  if(digitalRead(L_BLINKER_PIN)==HIGH){
    L_PIN_PRESS_COUNT++;
  }
  else{L_PIN_PRESS_COUNT=0;}

  if(R_PIN_PRESS_COUNT>BLINKER_PRESS_COUNT){
    R_BLINK_STATE = -R_BLINK_STATE;
    R_PIN_PRESS_COUNT = 0;
  }
  if(L_PIN_PRESS_COUNT>BLINKER_PRESS_COUNT){
    L_BLINK_STATE = -L_BLINK_STATE;
    L_PIN_PRESS_COUNT = 0;
  }

  blinkerThread.check();
  //PEDAL_RPM = PEDAL_TIME
  
  noInterrupts();
  uint32_t WHEEL_RPM = WHEEL_HE_COUNT*60000/(millis()-WHEEL_TIME);
  WHEEL_HE_COUNT = 0;
  WHEEL_TIME = millis();
  interrupts();

  #ifdef DEBUG
    printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
    printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
    printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
    printDateTime(gps.date, gps.time);
    printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  
    Serial.print("  || "); 
    Serial.print(R_BLINK_STATE);Serial.print(", ");
    Serial.print(L_BLINK_STATE);Serial.print(", ");
    //Serial.print(PEDAL_TIME);Serial.print(", "); 
    Serial.print(digitalRead(WHEEL_HE_PIN));Serial.print("  || "); 
  
    Serial.print("X: ");
    Serial.print(event.acceleration.x);
    Serial.print("  ");
    Serial.print("Y: ");
    Serial.print(event.acceleration.y);
    Serial.print("  ");
    Serial.print("Z: ");
    Serial.print(event.acceleration.z);
    Serial.print("  ");
    Serial.print("m/s^2 || ");
  
    Serial.print("X: ");
    Serial.print(event.magnetic.x);
    Serial.print("  ");
    Serial.print("Y: ");
    Serial.print(event.magnetic.y);
    Serial.print("  ");
    Serial.print("Z: ");
    Serial.print(event.magnetic.z);
    Serial.print("  ");
    Serial.println("uT");
  #endif
  
  delay(100);
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
