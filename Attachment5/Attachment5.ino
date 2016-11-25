/********************************************************************************************************
  ES52 Lab 6 (uController 8 Ball)
  <Part 5a Pushbutton Skeleton Code>
  
  By: Niamh Mulholland & Cynthia Gu
  Date: 11/23/16
  MRU:  11/24/16

  This code implements an interrupt version of the uController 8 Ball.
  
  Pressing the button starts random LEDs to light one at a time. 
  The button must be released and pressed again to keep the display
  changing.Once the button is released the rate begins to slow 
  down until only a single LED remains lit, displaying a prediction.  
  After several seconds, all LEDs go dark. 
  
  This program is implemented as a Finite State Machine with
  a cycle time of FSM_FREQ.
  
  In this skeleton program, values you need to select/add are indicated by "<?>"
********************************************************************************************************/
// CONSTANT declarations

// The following constant can be set TRUE to output debugging information to the serial port
const boolean debug = true;       // If TRUE debugging information sent to the serial port

// Define time delay constants - they can be adjusted to get the best display

const int FSM_FREQ = 20;             // frequency (in Hz) of the FSM
const int ROLL_DELAY = 50000;           // roll time (in Sec) after button released
const int DISP_DELAY = 4000;           // time to display final value (in Sec) before blanking display

// Define I/O Pin Constants here

const int pushButton = 1;
const int dataPin = 4;
const int clockPin = 5;
const int latchPin = 6;
const int pwrPin = 7;

  
// Declare global variables here

int curState;              // FSM state variable
                           // 1: Idle - waiting for start command; all LEDs off
long LEDnum = 0;
long prevNum = 0;
int rollCount = 0;
int dispCount = 0;

volatile boolean start = 0;    // ISR flag variable - set when button pressed

                      
               
                              
/*********************************************************
 * void setup()
 *
 * Runs once. Used to initialize variables
 * and set up I/O pins.
 *********************************************************/
void setup()
{

  // Initialize I/O pins here
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(pwrPin, OUTPUT);
  pinMode(pushButton, INPUT_PULLUP);
  
  digitalWrite(pwrPin, LOW);

  attachInterrupt(digitalPinToInterrupt(pushButton), checkStart, RISING);

  
   /* Set up serial port if debugging enabled */
 if (debug)
 {
   Serial.begin(9600);                 // This pipes serial print data to the serial monitor
   Serial.println("Initialization complete.");
 }
}


/*********************************************************
 * void turnLEDOn(int LED_Num)
 * 
 *
 * Function to turn on a specific M8B Display LED 
 * 
 * LED_Num = 1 (M8BD1; upper left)
 *  to
 * LED_Num = 12 (M8BD12; lower right)
 * 
 * LED_Num = 0  turns all LEDs off
 *********************************************************/


void turnLEDOn(int LED_Num)
{
   digitalWrite(latchPin, 0);
  
  int firstByte;
  int secondByte;
 
  if(LED_Num == 0){
    firstByte = 0;
    secondByte = 0;
  }
  else if (LED_Num < 9){
    firstByte = pow(2,LED_Num - 1) + .5;
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

boolean isRollEnd(void)
{
  rollCount++;
  if(rollCount >= ROLL_DELAY)
  {
    return true;
  }
  else
  {
    return false;
  }
}

boolean isDispEnd()
{
  dispCount++;
  if(dispCount >= DISP_DELAY)
  {
    return true;
  }
  else 
  {
    return false;
  }
}

/*********************************************************
 * boolean checkStart()
 * 
 * Function to see if TEST pushbutton is active
 * 
 * It might seem silly to have this simple test in a function, 
 * but isolating it in a function will make it easy to switch
 * in the more complicated accelerometer later without changing
 * the FSM.  In addition it makes replacing polled operation
 * with interrupt operation easier as well.
 * 
 * Returns TRUE if pushbutton pressed; FALSE otherwise
 *********************************************************/

boolean checkStart()
{
  start = 1;
  return start;
  start = 0;
}
 
/*********************************************************
 * Functions to simulate sleep and wake modes
 * 
 * void sleep()
 * 
 * sleep() powers down the TLC5916 shift registers and
 * turns off the green LED to simulate low power sleep
 * mode.
 * 
 * This function should:
 *      Turn off all uController LEDs
 *      Set all I/O pins connected to the 5916 low
 *      Turn off power to the 5916 display drivers
 *      Turn off the Arduino green LED to indicate sleep mode
 *      Call checkStart() to clear the Interrupt Service Routine flag
 *         This simulates the µController sleep mode
 * 
 * void wake()
 * 
 * wake() turns the TLC5916 shift registers back on and
 * turns on the green LED to indicate wake mode.
 * 
 * This function should:
 *      Turn on power to the 5916 display drivers
 *      Turn on the Arduino green LED to indicate wake mode
 *
 * We cannot actually put the Arduino Micro to sleep because
 * 1) the power LED draws about 1mA no matter what else we 
 * do and 2) if you do not do it right it is difficult to
 * program the Arduino to fix it.  So we simulate it.
 *********************************************************/

void sleep()
{
  digitalWrite(pwrPin, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  checkStart(); 

}

void wake()
{
  digitalWrite(pwrPin, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
}


/*********************************************************
 * void loop())
 *  
 * State Machine Loop
 * 
 * Uses the switch/case statement to only execute the code for the current state;
 * figures out what the next state is and continues the main program loop. 
 *
 * Five states shown.  You may need more or fewer.
 *  
 * Runs forever
 *********************************************************/

void loop()
{
  
  if (debug)                            // if debug enabled, print state to serial monitor
  {
    Serial.print("State: ");
    Serial.println(curState);
    Serial.print(checkStart());
  }

  switch (curState)
  {
    case 1:                             // Waiting for pushbutton 
    if(checkStart())
    {
      curState = 2;
    }
    else 
    {
      curState = 1;
    }

       break;

    case 2:
      wake();
      
      prevNum = LEDnum;      //set previous LEDNum so we can check for repeated num
      LEDnum = random(1,13);

      if(LEDnum == prevNum)
      {
        curState = 2;
      }
      else 
      {
        turnLEDOn(LEDnum); 
        curState = 3;                         
      }

         break;

    case 3: 

      if(!isRollEnd())   //if the button hasn't been pushed and the roll delay is not finished then stay in state 2
      { 
        curState = 2;
      }
      else if(checkStart())                                //if the button is pushed again go to state 4
      {
        rollCount = 0;
        curState = 4; 
      }
 
       break;  

    case 4:
      if(checkStart())    // if the putton is pushed, return to state 2     
      {
        curState = 2;                           
      }
      else                // start display time out
      { 
        while(!isDispEnd())
        {
          turnLEDOn(LEDnum);
        }
        curState = 5;     
        dispCount = 0;
      }
 
       break;     
       
    case 5: 
      turnLEDOn(0);
      sleep();
      curState = 1;  Serial.print("RESET");                            
 
       break; 

       default:
       {
          curState = 1;
       }

  }

  // End of state case statement
  // Here do anything that always gets done once per FSM cycle
 
  
  delay(1000/FSM_FREQ);                 // wait for next state machine clock cycle
}



