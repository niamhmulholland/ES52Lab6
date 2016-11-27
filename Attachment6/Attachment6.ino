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

// ISR flag
volatile boolean start;

void setup()
{
  pinMode(l1_interrupt, INPUT);

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
   if(!digitalRead(l1_interrupt))
   {
    start = true;
   }
   if(checkStart())
   {
    digitalWrite(LED_BUILTIN, HIGH); //turn led on
    readRegister(0x16);  //clear the register     
   }
   else
   {
    digitalWrite(LED_BUILTIN, LOW); //turn led off
   }
   delay(50);                   
}
