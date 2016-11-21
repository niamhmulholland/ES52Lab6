/*****************************************
 * void turnLEDon(int LED_Num)
 * 
 * Function to turn on a specific M8B Display LED
 * 
 * LED_Num = 1 (upper left)
 *                        to
 * LED_Num = 12 (lower right)
 * 
 * LED_Num = 0 turns all LEDs off
 *******************************************/
  //Pin connected to ST_CP of 74HC595
  const int latchPin = 6;
  //Pin connected to SH_CP of 74HC595
  const int clockPin = 5;
  //Pin connected to DS of 74HC595
  const int dataPin = 4;
  //time to light LED, lab part 4
  const int onTime = 500;
  const int pwrPin = 7;

  int LEDnum = 0;
  
  int curState;                 // holds current/next state
  int ooCount;                  // calculated value: number of state machine cycles to turn LED on and off
  int ooCtr;                    // counter for LED on and off time

  const int fsmFreq = 2;

void setup() {
  curState = 1;                         // initialize for first state in the program loop
  ooCount = 3;                        // calculate number of state machine cycles for LED on and off
  
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(pwrPin, OUTPUT);
  
  digitalWrite(pwrPin, LOW);
}

void turnLEDOn(int LED_Num){
  digitalWrite(latchPin, 0);
  
  int firstByte;
  int secondByte;
 
  if(LED_Num == 0){
    firstByte = 0;
    secondByte = 0;
  }
  else if (LED_Num < 9){
    firstByte = pow(2,LED_Num - 1)+ .5;
    secondByte = 0;
  }
  else {
    firstByte= 0;
    secondByte = pow(2,(LED_Num - 9)) + .5;
  }

  shiftOut(dataPin, clockPin, MSBFIRST, byte(firstByte));
  shiftOut(dataPin, clockPin, MSBFIRST, byte(secondByte));

  digitalWrite(latchPin, 1);
}

// When we turn the LED on/off in the blink mode,
// we need to wait ooCount cycles before moving to 
// the off/on state.  This function initializes the
// counter which determines when that delay ends.
void initOODly(void)
{
   ooCtr = 0;
}

// This function increments the counter and sees
// if the on/off delay is complete
boolean isDlyEnd(void)
{
   ooCtr++;
   if (ooCtr >= ooCount)
      return true;
   else
      return false;
}

void loop() {

    Serial.begin(500);
    
    switch(curState)
      {
        case 1: 
          Serial.print(LEDnum);
          Serial.print("\n");
          turnLEDOn(LEDnum);            
          if(isDlyEnd()&& LEDnum < 13){
            LEDnum++;
            initOODly(); 
          }
          else if(isDlyEnd()){

            LEDnum = 0;
            initOODly(); 
          }
        break;             
      }
    delay(1000/fsmFreq);
}
