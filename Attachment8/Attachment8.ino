/* Lab 7 MMA8451 Initialization Example

   By:   Niamh Mulholland & Cynthia Gu
   Date: 11/25/16
   mru:  11/25/16
          
   
   Program to demonstrate initializing the MMA8451
   accelerometer using the Arduino Wire library.  
   
   1) Reset the MMA8451.
   2) Read the MMA8451 WHO_AM_I register which should contain 0x1a.
   3) Write to the CTRL_REG1 register to set the data rate
   to 12.5Hz.
   4) Write to the CTRL_REG_2 to set low power mode.
   5) Set the MMA8451 to Active mode
  
   This reduces the supply current significantly from 
   from (165uA) used by the default value
   of 800Hz and normal mode to only 6uA.
   
   Sends status messages to the serial monitor.   
*/

#include <Wire.h>

int     MMA8451_ADDRESS = 0x1D;        //I2C address of accelerometer

/* MMA8451 Registers */

int     MMA8451_REG_WHOAMI = 0x0D;
int     MMA8451_REG_CTRL_REG1 = 0x2A;
int     MMA8451_REG_CTRL_REG2 = 0x2B;
int     MMA8451_REG_CTRL_REG4 = 0x2D;
int     MMA8451_REG_CTRL_REG5 = 0x2E;


/* MMA8451 register values */

// CTRL_REG1 values
int     MMA8451_DATARATE_12_5_HZ = 0b101;
int     MMA8451_ACTIVE = 0x01;            // set Active mode (vs Standby)

// CTRL_REG2 values
int     MMA8451_MODS_LP = 0b11;          // sets low power oversampling mode

byte    i;                               // general purpose variable


// accelerometer interrupt pin
int l1_interrupt = 1;
int led_pin = 13;
int lightCount = 0;
const int ON_TIME = 100;
// ISR flag
volatile boolean start = false;

// CONSTANT declarations
// The following constant can be set TRUE to output debugging information to the serial port
const boolean debug = true;       // If TRUE debugging information sent to the serial port
// Define time delay constants - they can be adjusted to get the best display
const int FSM_FREQ = 20;             // frequency (in Hz) of the FSM
const int ROLL_DELAY = 20;           // roll time (in Sec) after button released
const int DISP_DELAY = 25;           // time to display final value (in Sec) before blanking display
// Define I/O Pin Constants here
const int pushButton = 1;
const int dataPin = 4;
const int clockPin = 5;
const int latchPin = 6;
const int pwrPin = 7;
long LEDnum = 0;
long prevNum = 0;
int rollCount = 0;
int dispCount = 0;
int slowDown = 0;
  
// Declare global variables here
int curState;              // FSM state variable
                           // 1: Idle - waiting for start command; all LEDs off

void setup()
{
  // Initialize I/O pins here
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(pwrPin, OUTPUT);
  pinMode(pushButton, INPUT_PULLUP);
  
  digitalWrite(pwrPin, LOW);
  pinMode(l1_interrupt, INPUT);
  pinMode(led_pin, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(l1_interrupt), isInterrupt, FALLING);
  
 /* Initialize the wire library */
 Wire.begin();
  
 /* Set up serial port for output */
 Serial.begin(9600);               // This sends serial print data to the serial monitor
 delay(4000);                      // Add a delay to allow time to open serial monitor
                                   // to see initialization serial port status messages.
                                   // Remove for normal operation
  
  
 // Reset the MMA8451 by setting the reset bit in control register 2
 // and wait until the bit is cleared (which lets us know the reset is complete)
 //
 // Include a 1 sec timeout so we do not get stuck in an endless loop if the
 // accelerometer dies or is missing.

 writeRegister(MMA8451_REG_CTRL_REG2, 0x40);               // this resets the accelerometer
 i = 100;
 
 while (readRegister(MMA8451_REG_CTRL_REG2) & 0x40)        // wait for reset bit to be cleared
 {
   delay(10);
   if (i-- == 0) break;                                    // but give up after 1 sec
 }

 if (i == 0)
    Serial.println("MMA8451 reset failed.");
 else
 {
    Serial.println("MMA8451 reset.");
    
    // read the WHO_AM_I register to make sure we have a working accelerometer
    if ((i = readRegister(MMA8451_REG_WHOAMI)) == 0xFF)
    {
      Serial.println("No response to I2C read request.");
    }
    else
    {
      Serial.print("MMA8451 WHO_AM_I register returned: ");
      Serial.print(i, HEX);
      Serial.println("  (0x1A expected).");
    
    // Next we want to set a slower data rate to save power.
    // We also want to put the accelerometer in standby mode while we
    // configure it. We do this by writing to control register 1.
    
    // To make sure it worked, we read the register and print the new value
    // to the serial monitor.
             
      i = (MMA8451_DATARATE_12_5_HZ << 3 & ~MMA8451_ACTIVE);   // set 12.5Hz data rate, STANDBY mode
      writeRegister(MMA8451_REG_CTRL_REG1, i);
      delay(1);                                                 // seems we cannot read right away, gives occasional errors w/o this
      i = readRegister(MMA8451_REG_CTRL_REG1);
      Serial.print("MMA8451_REG_CTRL_REG1 set to: ");
      Serial.println(i, HEX);
    
      writeRegister(MMA8451_REG_CTRL_REG2, (MMA8451_MODS_LP << 3 | MMA8451_MODS_LP));      // this sets low power mode oversampling mode
      
     // ******************* initialization start **********************************
     // do initialization for operating modes and interrupts here
     // writeRegister(MMA8451_REG_CTRL_REG1, 0x18); 
      //set configuration register for motion detection
      writeRegister(0x15, 0xD8);
      //threshold setting value for Motion detection
      writeRegister(0x17, 0x04);
      //set debounce counter to eliminate false readings
      writeRegister(0x18, 0x02);
      //enable motion freefall/interrupt function in the system
      writeRegister(0x2D, 0x04);
      //route the motion/freefall interrupt function to INT1 hardware pint
      writeRegister(0x2E, 0x04);
     // writeRegister(MMA8451_REG_CTRL_REG1,0x01);
            
     // ******************** initialization end ***********************************
      
      i = (MMA8451_DATARATE_12_5_HZ << 3 | MMA8451_ACTIVE);   // set 12.5Hz data rate, ACTIVE mode
      writeRegister(MMA8451_REG_CTRL_REG1, i);  
   }
 }
}

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

void sleep()
{
  /*digitalWrite(pwrPin, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  checkStart(); */
}
void wake()
{
  /*digitalWrite(pwrPin, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);*/
}


void isInterrupt(void) 
{
  start = true; Serial.print("hi");
}

boolean checkStart()
{
  if(start)
  {
    start = !start; 
    return true;
  }
  else 
  {
    return false;
  }
}

//************************************************************************************
// Subroutine to read the value of a MMA8451 register
// Returns 0xff if error (which might be a problem if 0xff is a valid register value)
//************************************************************************************

byte readRegister(byte reg)
{
  Wire.beginTransmission(MMA8451_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);           // MMA8451 uses repeated start so don't give up I2C bus

  Wire.requestFrom(MMA8451_ADDRESS, 1);
  if (Wire.available() == 0)
    return 0xFF;
  return (Wire.read());
}

//************************************************************************************
// Subroutine to write a value to a MMA8451 register
//************************************************************************************

void writeRegister(byte reg, byte value)
{
  Wire.beginTransmission(MMA8451_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission(false);   
}

void loop()
{
  
  if (debug)                            // if debug enabled, print state to serial monitor
  {
    Serial.print("State: ");
    Serial.println(curState);
    //Serial.print(checkStart());
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
      else if (checkStart())
      {
        turnLEDOn(LEDnum); 
        curState = 2;                         
      }
      else
      {
        turnLEDOn(LEDnum); 
        curState = 3;  
        slowDown = slowDown + 8;
      }
      break;
    case 3: 
      if(checkStart())
      {
        curState = 2;
      }
      else if(ROLL_DELAY > rollCount) 
      { 
        rollCount++;
        curState = 2;
      }
      else
      {
        rollCount = 0;
        curState = 4; 
      }
 
       break;  
    case 4:
      if(checkStart())
      {
        curState = 2;
      }
      else if(DISP_DELAY > dispCount)
      { 
        turnLEDOn(LEDnum);
        dispCount++;
      }
      else
      {
        dispCount = 0;
        turnLEDOn(0);
        sleep();
        curState = 1;  Serial.print("RESET");   
      }                         
 
      break; 
    default:
    {
        curState = 1;
    }
  }
  // End of state case statement
  // Here do anything that always gets done once per FSM cycle
 
  
  delay(1000/FSM_FREQ + slowDown);                 // wait for next state machine clock cycle
}

