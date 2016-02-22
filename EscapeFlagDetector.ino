#include <Event.h>
#include <Timer.h>
/****** Preprocessor Macros *****/
// Debug settings
#define DEBUG_MODE 0

#define NONE 0
#define RED 1
#define ORANGE 2
#define GREEN 3
#define PURPLE 4
#define BLUE 5

// MAX_COLOR_TOLERANCE is the maximum allowed devation between the expected and observed 
// values across the three color channels to consider a color to be successfully detected
int MAX_COLOR_TOLERANCE = 15;

// These values are have been experimentally determined to be the average readings for the various flags
// For now, these are guesses

// row 0 corresponds to RED flag, row 1 to ORANGE, etc.
// col 0=red, col1=green, col2=blue
int flagColors[5][3] = {
 
  {85,  43,  44  }, // RED
 
  {101,  52,  45  }, // ORANGE
  
  {38,  49,  43  }, // GREEN
    
  {46,  48,  64  }, // PURPLE
 
  {38,  50, 70  } // BLUE
 
};

/***** GLOBAL VARIABLES *****/
const int buttonPin = 20;     // the number of the big red button pin
Timer t; // timer for recalibrating the sensors

long startTime;                    // start time for stop watch
long elapsedTime;                  // elapsed time for stop watch
long lastButtonPush;               // time or last valid button press
boolean timerActive = false;
const int debounceTime = 100;      // ignore button presses within this time window
int buttonCnt = 0;                 // counter for button events
const int timeWindow = 5000;       // you have a 5 second window to recalibrate the sensors 

// Color Sensor One (Looking for red)
int sensor1_s0=3;
int sensor1_s1=4;
int sensor1_s2=5;
int sensor1_s3=6;
int sensor1_out=2;
int sensor1_LED=11;

// Color Sensor Two (Looking for orange)
int sensor2_s0=22;
int sensor2_s1=24;
int sensor2_s2=26;
int sensor2_s3=28;
int sensor2_out=30;
int sensor2_LED=11;

// Color Sensor Three (Looking for green)
int sensor3_s0=32;
int sensor3_s1=34;
int sensor3_s2=36;
int sensor3_s3=38;
int sensor3_out=40;
int sensor3_LED=11;

// Color Sensor Four (Looking for purple)
int sensor4_s0=42;
int sensor4_s1=44;
int sensor4_s2=46;
int sensor4_s3=48;
int sensor4_out=50;
int sensor4_LED=11;

// Color Sensor Five (Looking for blue)
int sensor5_s0=23;
int sensor5_s1=25;
int sensor5_s2=27;
int sensor5_s3=29;
int sensor5_out=31;
int sensor5_LED=11;

// sensor statuses - initialized as all seeing no specific color
int sensorColors[] = {NONE, NONE, NONE, NONE, NONE};

// Solution to puzzle
int sensorColorsSolution[] = {RED, ORANGE, GREEN, PURPLE, BLUE};

int sensor1_flag=0;
int sen1_counter=0;
int sensor_countR[5] = {0,0,0,0,0}; // red counts for sensors 1-5
int sensor_countG[5] = {0,0,0,0,0};
int sensor_countB[5] = {0,0,0,0,0};

void setup()
 {
 Serial.begin(115200);

 // initialize the pushbutton pin as an input:
 pinMode(buttonPin, INPUT);
 // Attach an interrupt to the ISR vector
 // Note that when using pin 21 the button has to be attached to ground instead of 5v 
 // because interrupt pins 20 and 21 are always high
 attachInterrupt(digitalPinToInterrupt(buttonPin), pin_ISR, CHANGE);
 
 pinMode(sensor1_s0,OUTPUT);
 pinMode(sensor1_s1,OUTPUT); 
 pinMode(sensor1_s2,OUTPUT);
 pinMode(sensor1_s3,OUTPUT);
 pinMode(sensor1_LED,OUTPUT); 
 // Sensor 2
 pinMode(sensor2_s0,OUTPUT);
 pinMode(sensor2_s1,OUTPUT); 
 pinMode(sensor2_s2,OUTPUT);
 pinMode(sensor2_s3,OUTPUT);
 pinMode(sensor2_LED,OUTPUT); 
 // Sensor 3
 pinMode(sensor3_s0,OUTPUT);
 pinMode(sensor3_s1,OUTPUT); 
 pinMode(sensor3_s2,OUTPUT);
 pinMode(sensor3_s3,OUTPUT);
 pinMode(sensor3_LED,OUTPUT); 
 // Sensor 4
 pinMode(sensor4_s0,OUTPUT);
 pinMode(sensor4_s1,OUTPUT); 
 pinMode(sensor4_s2,OUTPUT);
 pinMode(sensor4_s3,OUTPUT);
 pinMode(sensor4_LED,OUTPUT); 
 // Sensor 5
 pinMode(sensor5_s0,OUTPUT);
 pinMode(sensor5_s1,OUTPUT); 
 pinMode(sensor5_s2,OUTPUT);
 pinMode(sensor5_s3,OUTPUT);
 pinMode(sensor5_LED,OUTPUT); 
}
void TCS()
{
   digitalWrite(sensor1_s1,HIGH);
   digitalWrite(sensor1_s0,LOW);
   sensor1_flag=0;
   attachInterrupt(0, ISR_INTO, CHANGE);
   timer_for_sensor_1_init();
}
void ISR_INTO()
{
  sen1_counter++;
}
 
void timer_for_sensor_1_init(void)
{
   TCCR2A=0x00;
   TCCR2B=0x07; //the clock frequency source 1024 points
   TCNT2= 100;    //10 ms overflow again
   TIMSK2 = 0x01; //allow interrupt
}

ISR(TIMER2_OVF_vect)//the timer 2, 10ms interrupt overflow again. Internal overflow interrupt executive function
{
  
 TCNT2=100;
 sensor1_flag++;
 if(sensor1_flag==1)
 {
    sen1_counter=0;
 }
 else if(sensor1_flag==2)
 {
  digitalWrite(sensor1_s2,LOW);
  digitalWrite(sensor1_s3,LOW); 
  sensor_countR[0]=sen1_counter/1.051;
  digitalWrite(sensor1_s2,HIGH);
  digitalWrite(sensor1_s3,HIGH);   
 }
 else if(sensor1_flag==3)
  {
   sensor_countG[0]=sen1_counter/1.0157;
   digitalWrite(sensor1_s2,LOW);
   digitalWrite(sensor1_s3,HIGH); 
 
  }
 else if(sensor1_flag==4)
 {
   sensor_countB[0]=sen1_counter/1.114;
   digitalWrite(sensor1_s2,LOW);
   digitalWrite(sensor1_s3,LOW);
 }
 else
 {
   sensor1_flag=0; 
   TIMSK2 = 0x00;
  }
  
sen1_counter=0; // reset counter
}

void setColorForSensor(int sensorNumber, int red, int green, int blue)
{
  
  // This function takes in the RGB values of a sensor and attempts to assign a color.
  // If the detected color isn't close enough to one of the solution colors, then 
  // the color is set to NONE. 
  // If mutliple matches are found, then the strongest match found is returned
   
  int absDiff = 0; // variable to hold the absolute difference between the expected value of the color and the observed
  int bestAbsDiff = 1000; // max difference is 255*3
   
  int closestMatch = NONE; // initialize as no color found

  // RED
  absDiff = abs(red - flagColors[0][0]) + abs(green - flagColors[0][1]) + abs(blue - flagColors[0][2]);
  closestMatch = closestMatchingColor(absDiff, bestAbsDiff, RED, closestMatch);
  if(DEBUG_MODE)
  {
      Serial.print("Abs diff for red is:");
      Serial.print(absDiff);
      Serial.print("Closest match after red:");
      Serial.println(closestMatch);
  }

  // ORANGE   
  absDiff = abs((int)red - flagColors[1][0]) + abs((int)green - flagColors[1][1]) + abs((int)blue - flagColors[1][2]);
  closestMatch = closestMatchingColor(absDiff, bestAbsDiff, ORANGE, closestMatch);
  if(DEBUG_MODE)
  {
      Serial.print("Abs diff for orange is:");
      Serial.print(absDiff);
      Serial.print("Closest match after orange:");
      Serial.println(closestMatch);
  }

  // GREEN
  absDiff = abs((int)red - flagColors[2][0]) + abs((int)green - flagColors[2][1]) + abs((int)blue - flagColors[2][2]);
  closestMatch = closestMatchingColor(absDiff, bestAbsDiff, GREEN, closestMatch);
  if(DEBUG_MODE)
  {
      Serial.print("Abs diff for green is:");
      Serial.print(absDiff);
      Serial.print("Closest match after blue:");
      Serial.println(closestMatch);
  }

  // PURPLE
  absDiff = abs((int)red - flagColors[3][0]) + abs((int)green - flagColors[3][1]) + abs((int)blue - flagColors[3][2]);
  closestMatch = closestMatchingColor(absDiff, bestAbsDiff, PURPLE, closestMatch);
  if(DEBUG_MODE)
  {
      Serial.print("Abs diff for purple is:");
      Serial.print(absDiff);
      Serial.print("Closest match after purple:");
      Serial.println(closestMatch);
  }

  // BLUE
  absDiff = abs((int)red - flagColors[4][0]) + abs((int)green - flagColors[4][1]) + abs((int)blue - flagColors[4][2]);
  closestMatch = closestMatchingColor(absDiff, bestAbsDiff, BLUE, closestMatch);
  if(DEBUG_MODE)
  {
      Serial.print("Abs diff for blue is:");
      Serial.print(absDiff);
      Serial.print("Closest match after green:");
      Serial.println(closestMatch);
  }

  // Subtract 1 from sensor number to get position in array
  sensorColors[sensorNumber - 1] = closestMatch;
  
  return; 
}

//int rollingAbsDiff(int expVal, int obsVal, int maxRange)
//{
//  // This function take in an experimental value and an observed value and returns the difference
//  // of the shortest rolling difference from eihter direction (example, if maxRange = 255, the obsVal is 254, and the expVal is 3, then
//  // the return would be 5 instead of 252)
//  
//  // THIS FUNCTION IS WRONG!! Can't simply subtract. Need to test in isolation before rolling in. 
//  
//  int leftDiff;
//  int rightDiff;
//  
//  leftDiff = expVal - obsVal;
//  rightDiff = obsVal - expVal;
//  
//  if(leftDiff < 0)
//  {
//    leftDiff+=maxRange;  
//  }
//  
//  if(rightDiff < 0)
//  {
//    rightDiff+=maxRange;  
//  }
//  
//  if(leftDiff < rightDiff)
//  {
//    return leftDiff;  
//  }
//  else
//  {
//    return rightDiff;  
//  }
//  
//}

int closestMatchingColor(int absDiff, int &bestAbsDiff, int COLOR, int currentBestMatch)
{
  // Function takes in the absdiff between the curretn color and a solution color, as well as the
  // color under consideration. If the values are within the qualification threshold, then the color passed
  // in is returned. Otherwise, NONE is returned.
  
  // If a better match has already been found, do not update the best matched color
  if (absDiff < MAX_COLOR_TOLERANCE)
  {    
    // One of the solution colors has been detected
    if(absDiff < bestAbsDiff)
    {
      // this is the best match found so far
      bestAbsDiff = absDiff; // as bestAbsDiff is passed by reference, it will be updated by the function
      return COLOR;
    }
    else
    {
      return currentBestMatch;  
    }
  }
  else
  {
    return currentBestMatch;  
  }
}
boolean isCorrectSolution()
{
   // This function checks the detected colors against the solution to the puzzle
   // and returns true if the puzzle has been solved
   
   if(!(sensorColors[0] == sensorColorsSolution[0]))
   {
     return false;
   }
   if(!(sensorColors[1] == sensorColorsSolution[1]))
   {
     return false;
   }
   if(!(sensorColors[2] == sensorColorsSolution[2]))
   {
     return false;
   }
   if(!(sensorColors[3] == sensorColorsSolution[3]))
   {
     return false;
   }
   if(!(sensorColors[4] == sensorColorsSolution[4]))
   {
     return false;
   }
   
   // All sensors are reading the correct colors
   return true;
}

void pin_ISR() {
  if(!timerActive)
  {
    startTime = millis();  
    lastButtonPush = startTime;
    timerActive = true;
    if(DEBUG_MODE)
    {
      Serial.println("Timer started.");
    }
    int dunno = t.after(timeWindow, timerExpired);
    buttonCnt = 1;
  }
  else
  {
    elapsedTime = millis()-lastButtonPush;
    
    if(elapsedTime > debounceTime)
    {
      buttonCnt++;
      lastButtonPush = millis();
      if (buttonCnt % 2 == 0)
      {
        if(!DEBUG_MODE)
        {
          Serial.print("Button has been pressed ");
          Serial.print(buttonCnt/2);
          Serial.println(" times.");
          Serial.print("Button pushed! elapsed time =");
          Serial.println(elapsedTime);
        }
        if(buttonCnt > 8)
        {
          // Button has been pressed 4 times in 5 seconds - recalibrate sensors
          recalibrateSensors();
          timerActive = false; 
        }
      }
    }
  }
}

void recalibrateSensors()
{
  for(int i = 0; i < 5; i++)
  {
    flagColors[i][0] = sensor_countR[i];
    flagColors[i][1] = sensor_countG[i];
    flagColors[i][2] = sensor_countB[i];
  }
  Serial.println("RECALIBRATED!**************");
  return;
}

void timerExpired()
{
  timerActive = false;
  Serial.println("Timer expired."); 
}

void loop()
 {
  delay(10);
  TCS();
  setColorForSensor(1, (int)sensor_countR[0], (int)sensor_countG[0], (int)sensor_countB[0]);
  printColor(sensorColors[0]);
  if(DEBUG_MODE)
  {
     if(sensorColors[0] == RED)
     {
        Serial.println("*******************************************");
        Serial.println("*******************************************");
        Serial.println("*******************************************");
        Serial.println("\n\n!!!RED FLAG DETECTED IN FRONT OF SENSOR!!!\n\n");
        Serial.println("*******************************************");
        Serial.println("*******************************************");
        Serial.print("red=");
        Serial.println(sensor_countR[0],DEC);
        Serial.print("green=");
        Serial.println(sensor_countG[0],DEC);
        Serial.print("blue=");
        Serial.println(sensor_countB[0],DEC);
     }
     
     else
     {
       Serial.print("\nRed flag not detected. Color found was:");
       Serial.println(sensorColors[0]);
       Serial.println("Where OTHER/NONE=0, RED=1, ORANGE=2, GREEN=3, PURPLE=4 and BLUE=5");
       Serial.print("red=");
       Serial.println(sensor_countR[0],DEC);
       Serial.print("green=");
       Serial.println(sensor_countG[0],DEC);
       Serial.print("blue=");
       Serial.println(sensor_countB[0],DEC);    
     }
   }
   delay(300);       
 }
 
 void printColor(int sensor)
 {
  
  Serial.print("*** Flag detected looks ");
  switch (sensor) 
  {
    case 1:
      Serial.println("RED ***");
      break;
    case 2:
      Serial.println("ORANGE ***");
      break;
    case 3:
      Serial.println("GREEN ***");
      break;
    case 4:
      Serial.println("PURPLE ***");
      break;
    case 5:
      Serial.println("BLUE ***");
      break;
    default: 
      Serial.println("n/a :( ***");
    break;
  } 
 }
