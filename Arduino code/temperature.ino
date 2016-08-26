/***************************************************************************

                     Copyright 2008 Gravitech
                        All Rights Reserved

****************************************************************************/

/***************************************************************************
 File Name: I2C_7SEG_Temperature.pde

 Hardware: Arduino Diecimila with 7-SEG Shield

 Description:
   This program reads I2C data from digital thermometer and display it on 7-Segment

 Change History:
   03 February 2008, Gravitech - Created

****************************************************************************/

#include <Wire.h> 
 
#define BAUD (9600)    /* Serial baud define */
#define _7SEG (0x38)   /* I2C address for 7-Segment */
#define THERM (0x49)   /* I2C address for digital thermometer */
#define EEP (0x50)     /* I2C address for EEPROM */
#define RED (3)        /* Red color pin of RGB LED */
#define GREEN (5)      /* Green color pin of RGB LED */
#define BLUE (6)       /* Blue color pin of RGB LED */

#define COLD (23)      /* Cold temperature, drive blue LED (23c) */
#define HOT (26)       /* Hot temperature, drive red LED (27c) */



#define echoPin 7 // Echo Pin
#define trigPin 8 // Trigger Pin
#define LEDPin 13 // Onboard LED

const byte NumberLookup[16] =   {0x3F,0x06,0x5B,0x4F,0x66,
                                 0x6D,0x7D,0x07,0x7F,0x6F, 
                                 0x77,0x7C,0x39,0x5E,0x79,0x71};
int maximumRange = 200; // Maximum range needed
int minimumRange = 0; // Minimum range needed
long duration, distance; // Duration used to calculate distance

/**************************
Flags for Different Signals
***************************/
int Celsius = 1;
int Arduino_Timer = 0;
int Standby = 0;
int Proximity = 0;
int counter = 0;
int incomingByte;
int time = 0;

/* Function prototypes */
void Cal_temp (int&, byte&, byte&, bool&);
void Dis_7SEG (int, byte, byte, bool);
void Send7SEG (byte, byte);
void SerialMonitorPrint (byte, int, bool);
void UpdateRGB (byte);

/***************************************************************************
 Function Name: setup

 Purpose: 
   Initialize hardwares.
****************************************************************************/


void setup() 
{ 
  //if (Proximity == 1){
     Serial.begin (9600);
     pinMode(trigPin, OUTPUT);
     pinMode(echoPin, INPUT);
     pinMode(LEDPin, OUTPUT); 
  //}
  //else{
    //Serial.begin(BAUD);
    Wire.begin();        /* Join I2C bus */
    pinMode(RED, OUTPUT);    
    pinMode(GREEN, OUTPUT);  
    pinMode(BLUE, OUTPUT);   
    delay(500); 
  //}
         /* Allow system to stabilize */
} 

/***************************************************************************
 Function Name: loop

 Purpose: 
   Run-time forever loop.
****************************************************************************/
 
void loop() 
{ 

  
  int Decimal;
  byte Temperature_H, Temperature_L, counter, counter2;
  bool IsPositive;
  
  /* Configure 7-Segment to 12mA segment output current, Dynamic mode, 
     and Digits 1, 2, 3 AND 4 are NOT blanked */
     
  Wire.beginTransmission(_7SEG);   
  byte val = 0; 
  Wire.write(val);
  val = B01000111;
  Wire.write(val);
  Wire.endTransmission();
  
  /* Setup configuration register 12-bit */
     
  Wire.beginTransmission(THERM);  
  val = 1;  
  Wire.write(val);
  val = B01100000;
  Wire.write(val);
  Wire.endTransmission();
  
  /* Setup Digital THERMometer pointer register to 0 */
     
  Wire.beginTransmission(THERM); 
  val = 0;  
  Wire.write(val);
  Wire.endTransmission();
  
  /* Test 7-Segment */
  for (counter=0; counter<8; counter++)
  {
    Wire.beginTransmission(_7SEG);
    Wire.write(1);
    for (counter2=0; counter2<4; counter2++)
    {
      Wire.write(1<<counter);
    }
    Wire.endTransmission();
    delay (250);
  }
  
  while (1)
  {
    


        if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
      if (incomingByte == 0){
        Proximity = 0;
      }
      if (incomingByte == 1){
        
      }
      if (incomingByte == 2){
        Proximity = 1;
        int i;
        for (i = 1; i <= 4; i++){
          Send7SEG(i, 0x40);
        }
      }
      if (incomingByte == 3){
      }
      if (incomingByte == 4){
        Standby = 1;
      }
      if (incomingByte == 5){
        Standby = 0;  
      }
      if (incomingByte == 6){
        Celsius = 1;
      }
      if (incomingByte == 7){
        Celsius = 0;
      }
    }

      if (Standby == 1){
        int i;
        for (i = 1; i <= 4; i++){
          Send7SEG(i, 0x40);
        }
        Serial.print("Standby Mode!");
        delay(1000);
      } 
       else if (Proximity == 1){
           digitalWrite(trigPin, LOW); 
           delayMicroseconds(2); 
          
           digitalWrite(trigPin, HIGH);
           delayMicroseconds(10); 
           
           digitalWrite(trigPin, LOW);
           duration = pulseIn(echoPin, HIGH);
           
           //Calculate the distance (in cm) based on the speed of sound.
           distance = duration/58.2;
           
           if (distance >= maximumRange || distance <= minimumRange){
           /* Send a negative number to computer and Turn LED ON 
           to indicate "out of range" */
           Serial.println("-1");
           digitalWrite(LEDPin, HIGH); 
           }
           else {
           /* Send the distance to the computer using Serial protocol, and
           turn LED OFF to indicate successful reading. */
           Serial.print("Distance is: ");
           Serial.print(distance);
           Serial.print("\n");
           digitalWrite(LEDPin, LOW); 
           }
           
           //Delay 50ms before next reading.
           delay(1000);
        
       }
       /*else if (Arduino_Timer == 1){
          counter++;
          delay(1000);
          Dis_Timer();
        
       }*/
      else{
         Wire.requestFrom(THERM, 2);
         Temperature_H = Wire.read();
         Temperature_L = Wire.read();


    /* Calculate temperature */
    Cal_temp (Decimal, Temperature_H, Temperature_L, IsPositive);
    
    /* Display temperature on the serial monitor. 
       Comment out this line if you don't use serial monitor.*/
    SerialMonitorPrint (Temperature_H, Decimal, IsPositive);
    
    /* Update RGB LED.*/
    UpdateRGB (Temperature_H);
    
    /* Display temperature on the 7-Segment */
    Dis_7SEG (Decimal, Temperature_H, Temperature_L, IsPositive);
    
    delay (1000);        /* Take temperature read every 1 second */
    }
  }
} 

/***************************************************************************
 Function Name: Cal_temp

 Purpose: 
   Calculate temperature from raw data.
****************************************************************************/
void Cal_temp (int& Decimal, byte& High, byte& Low, bool& sign)
{
  if ((High&B10000000)==0x80)    /* Check for negative temperature. */
    sign = 0;
  else
    sign = 1;
    
  High = High & B01111111;      /* Remove sign bit */
  Low = Low & B11110000;        /* Remove last 4 bits */
  Low = Low >> 4; 
  Decimal = Low;
  Decimal = Decimal * 625;      /* Each bit = 0.0625 degree C */
  
  if (sign == 0)                /* if temperature is negative */
  {
    High = High ^ B01111111;    /* Complement all of the bits, except the MSB */
    Decimal = Decimal ^ 0xFF;   /* Complement all of the bits */
  }  
}

/***************************************************************************
 Function Name: Dis_7SEG

 Purpose: 
   Display number on the 7-segment display.
****************************************************************************/
void Dis_7SEG (int Decimal, byte High, byte Low, bool sign)
{
  if (incomingByte == 1){
    counter++;
    Dis_Timer();
    
  } 
  else{
      counter = -1;
      byte Digit = 4;                 /* Number of 7-Segment digit */
  byte Number;                    /* Temporary variable hold the number to display */
   if (Celsius == 0) {
    double decTemp = Decimal;
    while (decTemp >= 1) {
      decTemp /= 10;
    }
    
    double cTemp = High + decTemp;
    /*if (!sign) {
      cTemp = cTemp * -1;
    }*/
    
    double fTemp = (cTemp * 9/5) + 32;
    sign = fTemp >= 0;
    High = ((byte) fTemp) ;
    decTemp = fTemp - High;
    Decimal = decTemp * 10000;  
  }
  
  if (sign == 0)                  /* When the temperature is negative */
  {
    Send7SEG(Digit,0x40);         /* Display "-" sign */
    Digit--;                      /* Decrement number of digit */
  }
  
  if (High > 99)                  /* When the temperature is three digits long */
  {
    Number = High / 100;          /* Get the hundredth digit */
    Send7SEG (Digit,NumberLookup[Number]);     /* Display on the 7-Segment */
    High = High % 100;            /* Remove the hundredth digit from the TempHi */
    Digit--;                      /* Subtract 1 digit */    
  }
  
  if (High > 9)
  {
    Number = High / 10;           /* Get the tenth digit */
    Send7SEG (Digit,NumberLookup[Number]);     /* Display on the 7-Segment */
    High = High % 10;            /* Remove the tenth digit from the TempHi */
    Digit--;                      /* Subtract 1 digit */
  }
  
  Number = High;                  /* Display the last digit */
  Number = NumberLookup [Number]; 
  if (Digit > 1)                  /* Display "." if it is not the last digit on 7-SEG */
  {
    Number = Number | B10000000;
  }
  Send7SEG (Digit,Number);  
  Digit--;                        /* Subtract 1 digit */
  
  if (Digit > 0)                  /* Display decimal point if there is more space on 7-SEG */
  {
    Number = Decimal / 1000;
    Send7SEG (Digit,NumberLookup[Number]);
    Digit--;
  }

  if (Digit > 0)                 /* Display "c" if there is more space on 7-SEG */
  {
    if (Celsius == 1) {

      Send7SEG (Digit,0x39);
    }
    else {

      Send7SEG (Digit,0x71);
    }
    Digit--;
  }
  
  if (Digit > 0)                 /* Clear the rest of the digit */
  {
    Send7SEG (Digit,0x00);    
  } 
    
  }
  
 
}

/***************************************************************************
 Function Name: Send7SEG

 Purpose: 
   Send I2C commands to drive 7-segment display.
****************************************************************************/

void Send7SEG (byte Digit, byte Number)
{
  Wire.beginTransmission(_7SEG);
  Wire.write(Digit);
  Wire.write(Number);
  Wire.endTransmission();
}

/***************************************************************************
 Function Name: UpdateRGB

 Purpose: 
   Update RGB LED according to define HOT and COLD temperature. 
****************************************************************************/

void UpdateRGB (byte Temperature_H)
{
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);        /* Turn off all LEDs. */
  
  if (Temperature_H <= COLD)
  {
    digitalWrite(BLUE, HIGH);
  }
  else if (Temperature_H >= HOT)
  {
    digitalWrite(RED, HIGH);
  }
  else 
  {
    digitalWrite(GREEN, HIGH);
  }
}

/***************************************************************************
 Function Name: SerialMonitorPrint

 Purpose: 
   Print current read temperature to the serial monitor.
****************************************************************************/
void SerialMonitorPrint (byte Temperature_H, int Decimal, bool IsPositive)
{
    Serial.print("Temperature: ");
    if (!IsPositive)
    {
      Serial.print("-");
    }
    Serial.print(Temperature_H, DEC);
    Serial.print(".");
    Serial.print(Decimal, DEC);
    Serial.print("\n");
}
   


/****************************************
This method displays time on 7Seg Display
*****************************************/

void Dis_Timer(){
  int thousand = 0, hundred = 0, ten = 0, one = 0, digit = 4;
  if (Standby == 1){
    int i;
    for (i = 1; i <= 4; i++){
        Send7SEG(i, 0x40);
    }
   Serial.print("Standby Mode!");
   return;
  }
  int number = counter;
  if (number > 9999){
    number = number % 10000;
  }

  thousand = number / 1000;
  number = number % 1000;

  hundred = number / 100;
  number = number % 100;

  ten = number / 10;
  number = number % 10;

  one = number;

  Send7SEG(digit, NumberLookup[thousand]);
  digit--;
  Send7SEG(digit, NumberLookup[hundred]);
  digit--;
  Send7SEG(digit, NumberLookup[ten]);
  digit--;
  Send7SEG(digit, NumberLookup[one]);
  digit--;  

}

