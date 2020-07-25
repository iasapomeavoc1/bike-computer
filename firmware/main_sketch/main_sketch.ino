#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Adafruit_LSM303_Accel.h>
#include <Adafruit_LSM303DLH_Mag.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

// TODO
// - Blinker behavior to neopixel, all inside arduino, status messages to raspi
// - Mag + Accel integration, status messages to raspi, Accel integration with neopixel (brake light)
// - Hall Effect sensor integration, functions for determining speed + cadence, status messages to raspi
// - Light sensor integration with neopixel
// - Temp sensor, pressure sensor, humidity sensor?? car proximity indicator??

#define DEBUG = true

// Pin defintion
static const int GPS_RX_PIN = 10;
static const int GPS_TX_PIN = 9;
static const int PEDAL_HE_PIN = 7;
static const int WHEEL_HE_PIN = 8;
static const int L_BLINKER_PIN = 4;
static const int R_BLINKER_PIN = 5;
static const int LED_PIN = 6;

// GPS setup
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(GPS_RX_PIN, GPS_TX_PIN);

// Variables to handle hall sensor tachometer
volatile uint32_t PEDAL_HE_COUNT;
volatile uint32_t WHEEL_HE_COUNT;
unsigned long PEDAL_TIME = 0;

// NeoPixel setup
static const int LED_COUNT = 40;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Accelerometer setup
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

// Magnetometer setup
Adafruit_LSM303DLH_Mag_Unified mag = Adafruit_LSM303DLH_Mag_Unified(12345);

void pedal_hall_toggle(){  
  PEDAL_HE_COUNT++;
}

void setup()
{ 
  pinMode(PEDAL_HE_PIN,INPUT); // Change pinmode for hall effect sensor digital pins to input.
  pinMode(WHEEL_HE_PIN,INPUT); 
  pinMode(L_BLINKER_PIN,INPUT); // Change pinmode for button digital pins to input.
  pinMode(R_BLINKER_PIN,INPUT);

  attachInterrupt(digitalPinToInterrupt(PEDAL_HE_PIN),pedal_hall_toggle,FALLING);
  
  strip.begin();           // Initialize NeoPixel strip object 
  strip.show();            // Turn OFF all pixels
  strip.setBrightness(50); // Set brightness to about 1/5 (max = 255)
  
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
  
  noInterrupts();
  uint32_t PEDAL_RPM = PEDAL_HE_COUNT*60000/(millis()-PEDAL_TIME);
  PEDAL_HE_COUNT = 0;
  PEDAL_TIME = millis();
  interrupts();

  if(DEBUG){
    printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
    printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
    printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
    printDateTime(gps.date, gps.time);
    printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  
    Serial.print("  || "); 
    Serial.print(digitalRead(L_BLINKER_PIN));Serial.print(", ");
    Serial.print(digitalRead(R_BLINKER_PIN));Serial.print(", ");
    Serial.print(PEDAL_TIME);Serial.print(", "); 
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
  }
  delay(100);

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
