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

// GPS setup
static const int GPS_RX_PIN = 10;
static const int GPS_TX_PIN = 9;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(GPS_RX_PIN, GPS_TX_PIN);

// Digital input pins from hall effect sensors
static const int L_BLINKER_PIN = 11;
static const int R_BLINKER_PIN = 12;

// Analog input pin from light sensor
static const int LIGHT_SENSOR_PIN = 1;

// NeoPixel setup
static const int LED_PIN = 6;
static const int LED_COUNT = 40;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Accelerometer setup
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

// Magnetometer setup
Adafruit_LSM303DLH_Mag_Unified mag = Adafruit_LSM303DLH_Mag_Unified(12345);

void setup()
{
  strip.begin();           // Initialize NeoPixel strip object 
  strip.show();            // Turn OFF all pixels
  strip.setBrightness(50); // Set brightness to about 1/5 (max = 255)
  
  ss.begin(GPSBaud); // Initialize GPS device

  accel.begin(); // Initialize accelerometer
  mag.begin(); // Initialize magnetometer

  pinMode(L_BLINKER_PIN,INPUT); // Change pinmode for hall effect sensor digital pins
  pinMode(R_BLINKER_PIN,INPUT); // to input.

  Serial.begin(115200);
}
void loop()
{
  // Output raw GPS data to the serial monitor
  while (ss.available() > 0){
    gps.encode(ss.read());
    delay(10);
    //Serial.write(ss.read());
    Serial.print(gps.satellites.value());Serial.print(", ");Serial.print(gps.hdop.hdop());Serial.print(", ");
    Serial.print(gps.location.lat());Serial.print(", ");Serial.print(gps.location.lng());Serial.print(", ");
    Serial.print(gps.location.age());Serial.print(", ");
    
    //Serial.print(gps.date());Serial.print(', ');Serial.print(gps.time());Serial.print(', ');
    printDateTime(gps.date,gps.time);Serial.print(", ");
    
    Serial.print(gps.altitude.meters());Serial.print(", ");Serial.print(gps.course.deg());Serial.print(", ");
    Serial.print(gps.speed.kmph());Serial.print(", ");Serial.println(TinyGPSPlus::cardinal(gps.course.deg()));
  }

  
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
}
