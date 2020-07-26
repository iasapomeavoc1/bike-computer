#define LED_BLACK    0

#define LED_RED_VERYLOW   (3 <<  11)
#define LED_RED_LOW     (7 <<  11)
#define LED_RED_MEDIUM    (15 << 11)
#define LED_RED_HIGH    (31 << 11)

#define LED_GREEN_VERYLOW (1 <<  5)   
#define LED_GREEN_LOW     (15 << 5)  
#define LED_GREEN_MEDIUM  (31 << 5)  
#define LED_GREEN_HIGH    (63 << 5)  

#define LED_BLUE_VERYLOW  3
#define LED_BLUE_LOW    7
#define LED_BLUE_MEDIUM   15
#define LED_BLUE_HIGH     31

#define LED_ORANGE_VERYLOW  (LED_RED_VERYLOW + LED_GREEN_VERYLOW)
#define LED_ORANGE_LOW    (LED_RED_LOW     + LED_GREEN_LOW)
#define LED_ORANGE_MEDIUM (LED_RED_MEDIUM  + LED_GREEN_MEDIUM)
#define LED_ORANGE_HIGH   (LED_RED_HIGH    + LED_GREEN_HIGH)

#define LED_PURPLE_VERYLOW  (LED_RED_VERYLOW + LED_BLUE_VERYLOW)
#define LED_PURPLE_LOW    (LED_RED_LOW     + LED_BLUE_LOW)
#define LED_PURPLE_MEDIUM (LED_RED_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_PURPLE_HIGH   (LED_RED_HIGH    + LED_BLUE_HIGH)

#define LED_CYAN_VERYLOW  (LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_CYAN_LOW    (LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_CYAN_MEDIUM   (LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_CYAN_HIGH   (LED_GREEN_HIGH    + LED_BLUE_HIGH)

#define LED_WHITE_VERYLOW (LED_RED_VERYLOW + LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_WHITE_LOW   (LED_RED_LOW     + LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_WHITE_MEDIUM  (LED_RED_MEDIUM  + LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_WHITE_HIGH    (LED_RED_HIGH    + LED_GREEN_HIGH    + LED_BLUE_HIGH)

static const uint8_t PROGMEM
    right_blinker_bmp[][8] =
          {
            {//0
              B11001000,
              B01100000,
              B00110000,
              B10011000,
              B10011000,
              B00110000,
              B01100000,
              B11001000,
            },
            {//1
              B01100100,
              B00110000,
              B10011000,
              B11001100,
              B11001100,
              B10011000,
              B00110000,
              B01100100,
            },
            {//2
              B00110010,
              B10011000,
              B11001100,
              B01100110,
              B01100110,
              B11001100,
              B10011000,
              B00110010,
            },
            {//3
              B10011001,
              B11001100,
              B01100110,
              B00110011,
              B00110011,
              B01100110,
              B11001100,
              B10011001,
            },
          };


static const uint8_t PROGMEM
    left_blinker_bmp[][8] =
          {
            {//0
              B10011001,
              B00110011,
              B01100110,
              B11001100,
              B11001100,
              B01100110,
              B00110011,
              B10011001,
            },
            {//1
              B00110011,
              B01100110,
              B11001100,
              B10011001,
              B10011001,
              B11001100,
              B01100110,
              B00110011,
            },
            {//2
              B01100110,
              B11001100,
              B10011001,
              B00110011,
              B00110011,
              B10011001,
              B11001100,
              B01100110,
            },
            {//3
              B11001100,
              B10011001,
              B00110011,
              B01100110,
              B01100110,
              B00110011,
              B10011001,
              B11001100,
            },
          };

static const uint8_t PROGMEM
    hazard_blinker_bmp[][8] =
          {
            {//0
              B00000000,
              B00000000,
              B00000000,
              B00000000,
              B00000000,
              B00000000,
              B00000000,
              B00000000,
            },
            {//1
              B00000000,
              B00000000,
              B00100000,
              B01110000,
              B01110000,
              B00100000,
              B00000000,
              B00000000,
            },
            {//2
              B00000000,
              B00100000,
              B01110000,
              B11011000,
              B11011000,
              B01110000,
              B00100000,
              B00000000,
            },
            {//3
              B00100000,
              B01110000,
              B11011000,
              B10001000,
              B10001000,
              B11011000,
              B01110000,
              B00100000,
            },
            {//4
              B01110000,
              B11011000,
              B10001000,
              B00000000,
              B00000000,
              B10001000,
              B11011000,
              B01110000,
            },            
            {//5
              B11011000,
              B10001000,
              B00000000,
              B00000000,
              B00000000,
              B00000000,
              B10001000,
              B11011000,
            },
          };
