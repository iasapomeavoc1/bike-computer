#define PEDAL_HE_PIN 8
#define WHEEL_HE_PIN 7
 
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
float PEDAL_TRIGGER_DELTAS[L_PTT] = {0,0,0,0,0,0,0,0,0,0};
int PTT_ARRAY_INDEX = 0;

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

void setup() { Serial.begin(115200); 
  pinMode(PEDAL_HE_PIN,INPUT); // Change pinmode for hall effect sensor digital pins to input.
  pinMode(WHEEL_HE_PIN,INPUT); 
  }

void loop() {
       
  pedal_hall_interrupt();
  Serial.print(PTT_ARRAY_INDEX);  Serial.print(",");

  for(int i=0; i<L_PTT; i++){
    Serial.print(PEDAL_TRIGGER_DELTAS[i]);  Serial.print(",");  
  }
  
  if(millis()-PEDAL_TRIGGER_TIME>2000){
    increment_pedal_array();
  }
  
  PEDAL_RPM = 0;
  if( !(PEDAL_TRIGGER_DELTAS[PTT_ARRAY_INDEX]>2000 && PEDAL_TRIGGER_DELTAS[PTT_ARRAY_INDEX-1]>2000 && PEDAL_TRIGGER_DELTAS[PTT_ARRAY_INDEX-2]>2000) ){
    for(int j=0; j<L_PTT; j++){
      PEDAL_RPM += 1.0/(PEDAL_TRIGGER_DELTAS[j])*60000;
    }
    PEDAL_RPM = PEDAL_RPM/L_PTT;
  }
    Serial.println(PEDAL_RPM);  

 }
