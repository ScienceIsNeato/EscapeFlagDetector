#include <Event.h>
#include <Timer.h>
#include <Keypad.h>
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
  {60,  29,  29    }, // RED
  {53,  32,  29  }, // ORANGE 
  {27,  34,  26  }, // GREEN    
  {35,  34,  42  }, // PURPLE 
  {29,  38, 50  } // BLUE
};

/***** GLOBAL VARIABLES *****/
boolean isDoorLocked = false;
const int buttonPin = 2;     // the number of the big red button pin

Timer motionSensorTimer;
Timer recalibrationTimer; // delay for recalibration
int motionTimerPID = 0;

long startTime;                    // start time for stop watch
long elapsedTime;                  // elapsed time for stop watch
long lastButtonPush;               // time or last valid button press
boolean timerActive = false;
const int debounceTime = 100;      // ignore button presses within this time window
int buttonCnt = 0;                 // counter for button events
const int timeWindow = 3000;       // you have a 5 second window to recalibrate the sensors 

// Color Sensor One (Looking for red)
int sensor1_s0=4;
int sensor1_s1=5;
int sensor1_s2=6;
int sensor1_s3=7;
int sensor1_out=3;

// Color Sensor Two (Looking for orange)
int sensor2_s0=22;
int sensor2_s1=24;
int sensor2_s2=26;
int sensor2_s3=28;
int sensor2_out=18;

// Color Sensor Three (Looking for green)
int sensor3_s0=30;
int sensor3_s1=32;
int sensor3_s2=34;
int sensor3_s3=36;
int sensor3_out=19;

// Color Sensor Four (Looking for purple)
int sensor4_s0=38;
int sensor4_s1=40;
int sensor4_s2=42;
int sensor4_s3=44;
int sensor4_out=21;

// Color Sensor Five (Looking for blue)
int sensor5_s0=46;
int sensor5_s1=48;
int sensor5_s2=50;
int sensor5_s3=52;
int sensor5_out=20;

// sensor statuses - initialized as all seeing no specific color
int sensorColors[] = {NONE, NONE, NONE, NONE, NONE};

// Solution to puzzle
int sensorColorsSolution[] = {RED, ORANGE, GREEN, PURPLE, BLUE};

int sensor1_flag=0;
int sen1_counter=0;
int sensor2_flag=0;
int sen2_counter=0;
int sensor3_flag=0;
int sen3_counter=0;
int sensor4_flag=0;
int sen4_counter=0;
int sensor5_flag=0;
int sen5_counter=0;
int sensor_countR[5] = {0,0,0,0,0}; // red counts for sensors 1-5
int sensor_countG[5] = {0,0,0,0,0};
int sensor_countB[5] = {0,0,0,0,0};

// door lock relay
int relayPin = 11; // don't use pin 12 - that's the reset pin!!!!
int recalibrated = false;

int ledPin = 13; // pin that controls the lights for the sensors

// KEYPAD
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {45, 47, 49, 51}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {53, 41, 43}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
int firstTime = true;

String keypadSequence = "0000000";

// Motion sensor
int motionSensorPin = 1;
int motionVal = 0;
const int motionSensorDetectionInterval = 250; // poll IR sensor every quarter second
const int motionThreshold = 200; // values above this will trigger door unlock
const int motionSensorDelayWindow=7000; // after the door is locked, ignore motion sensor actions for 7 seconds
boolean motionSensorActive = false;
long timeDoorLocked = 0;

// Recalibration Timer
const int recalibrationDelay = 3000; // let sensors sense for 3 seconds before recalibrating

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
 
 // Sensor 2
 pinMode(sensor2_s0,OUTPUT);
 pinMode(sensor2_s1,OUTPUT); 
 pinMode(sensor2_s2,OUTPUT);
 pinMode(sensor2_s3,OUTPUT);
 //pinMode(sensor2_LED,OUTPUT); 
 // Sensor 3
 pinMode(sensor3_s0,OUTPUT);
 pinMode(sensor3_s1,OUTPUT); 
 pinMode(sensor3_s2,OUTPUT);
 pinMode(sensor3_s3,OUTPUT);
 //pinMode(sensor3_LED,OUTPUT); 
 // Sensor 4
 pinMode(sensor4_s0,OUTPUT);
 pinMode(sensor4_s1,OUTPUT); 
 pinMode(sensor4_s2,OUTPUT);
 pinMode(sensor4_s3,OUTPUT);
 //pinMode(sensor4_LED,OUTPUT); 
 // Sensor 5
 pinMode(sensor5_s0,OUTPUT);
 pinMode(sensor5_s1,OUTPUT); 
 pinMode(sensor5_s2,OUTPUT);
 pinMode(sensor5_s3,OUTPUT);
 //pinMode(sensor5_LED,OUTPUT); 
 
 // Relay
 pinMode(relayPin, OUTPUT);
 digitalWrite(relayPin, LOW); //disconnect the relay 
 
 pinMode(ledPin, OUTPUT);
 analogWrite(ledPin, 175);
 
 motionSensorTimer.every(motionSensorDetectionInterval, checkMotionSensor);
}
void TCS()
{
   digitalWrite(sensor1_s1,HIGH);
   digitalWrite(sensor1_s0,LOW);
   digitalWrite(sensor2_s1,HIGH);
   digitalWrite(sensor2_s0,LOW);
   digitalWrite(sensor3_s1,HIGH);
   digitalWrite(sensor3_s0,LOW);
   digitalWrite(sensor4_s1,HIGH);
   digitalWrite(sensor4_s0,LOW);
   digitalWrite(sensor5_s1,HIGH);
   digitalWrite(sensor5_s0,LOW);
   
   sensor1_flag=0;
   sensor2_flag=0;
   sensor3_flag=0;
   sensor4_flag=0;
   sensor5_flag=0;
   
   attachInterrupt(digitalPinToInterrupt(sensor1_out), ISR_INTO_1, CHANGE);
   attachInterrupt(digitalPinToInterrupt(sensor2_out), ISR_INTO_2, CHANGE);
   attachInterrupt(digitalPinToInterrupt(sensor3_out), ISR_INTO_3, CHANGE);
   attachInterrupt(digitalPinToInterrupt(sensor4_out), ISR_INTO_4, CHANGE);
   attachInterrupt(digitalPinToInterrupt(sensor5_out), ISR_INTO_5, CHANGE);
   
   timer_for_sensor_1_init();
}

void ISR_INTO_1()
{
  sen1_counter++;
}
void ISR_INTO_2()
{
  sen2_counter++;
}
void ISR_INTO_3()
{
  sen3_counter++;
}
void ISR_INTO_4()
{
  sen4_counter++;
}
void ISR_INTO_5()
{
  sen5_counter++;
}
 
void timer_for_sensor_1_init(void)
{
   TCCR2A=0x00;
   TCCR2B=0x07; //the clock frequency source 1024 points
   TCNT2= 100;    //10 ms overflow again
   TIMSK2 = 0x01; //allow interrupt
}

ISR(TIMER2_OVF_vect, ISR_NOBLOCK)//the timer 2, 10ms interrupt overflow again. Internal overflow interrupt executive function
{
 getNumFromKeypad();
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

/*****************************************/
sensor2_flag++;
 if(sensor2_flag==1)
 {
    sen2_counter=0;
 }
 else if(sensor2_flag==2)
 {
  digitalWrite(sensor2_s2,LOW);
  digitalWrite(sensor2_s3,LOW); 
  sensor_countR[1]=sen2_counter/1.051;
  digitalWrite(sensor2_s2,HIGH);
  digitalWrite(sensor2_s3,HIGH);   
 }
 else if(sensor2_flag==3)
  {
   sensor_countG[1]=sen2_counter/1.0157;
   digitalWrite(sensor2_s2,LOW);
   digitalWrite(sensor2_s3,HIGH); 
 
  }
 else if(sensor2_flag==4)
 {
   sensor_countB[1]=sen2_counter/1.114;
   digitalWrite(sensor2_s2,LOW);
   digitalWrite(sensor2_s3,LOW);
 }
 else
 {
   sensor2_flag=0; 
   TIMSK2 = 0x00;
  }
  
sen2_counter=0; // reset counter
/*****************************************/
sensor3_flag++;
 if(sensor3_flag==1)
 {
    sen3_counter=0;
 }
 else if(sensor3_flag==2)
 {
  digitalWrite(sensor3_s2,LOW);
  digitalWrite(sensor3_s3,LOW); 
  sensor_countR[2]=sen3_counter/1.051;
  digitalWrite(sensor3_s2,HIGH);
  digitalWrite(sensor3_s3,HIGH);   
 }
 else if(sensor3_flag==3)
  {
   sensor_countG[2]=sen3_counter/1.0157;
   digitalWrite(sensor3_s2,LOW);
   digitalWrite(sensor3_s3,HIGH); 
 
  }
 else if(sensor3_flag==4)
 {
   sensor_countB[2]=sen3_counter/1.114;
   digitalWrite(sensor3_s2,LOW);
   digitalWrite(sensor3_s3,LOW);
 }
 else
 {
   sensor3_flag=0; 
   TIMSK2 = 0x00;
  }
  
sen3_counter=0; // reset counter
/*****************************************/
sensor4_flag++;
 if(sensor4_flag==1)
 {
    sen4_counter=0;
 }
 else if(sensor4_flag==2)
 {
  digitalWrite(sensor4_s2,LOW);
  digitalWrite(sensor4_s3,LOW); 
  sensor_countR[3]=sen4_counter/1.051;
  digitalWrite(sensor4_s2,HIGH);
  digitalWrite(sensor4_s3,HIGH);   
 }
 else if(sensor4_flag==3)
  {
   sensor_countG[3]=sen4_counter/1.0157;
   digitalWrite(sensor4_s2,LOW);
   digitalWrite(sensor4_s3,HIGH); 
 
  }
 else if(sensor4_flag==4)
 {
   sensor_countB[3]=sen4_counter/1.114;
   digitalWrite(sensor4_s2,LOW);
   digitalWrite(sensor4_s3,LOW);
 }
 else
 {
   sensor4_flag=0; 
   TIMSK2 = 0x00;
  }
  
sen4_counter=0; // reset counter

/*****************************************/
sensor5_flag++;
 if(sensor5_flag==1)
 {
    sen5_counter=0;
 }
 else if(sensor5_flag==2)
 {
  digitalWrite(sensor5_s2,LOW);
  digitalWrite(sensor5_s3,LOW); 
  sensor_countR[4]=sen5_counter/1.051;
  digitalWrite(sensor5_s2,HIGH);
  digitalWrite(sensor5_s3,HIGH);   
 }
 else if(sensor5_flag==3)
  {
   sensor_countG[4]=sen5_counter/1.0157;
   digitalWrite(sensor5_s2,LOW);
   digitalWrite(sensor5_s3,HIGH); 
 
  }
 else if(sensor5_flag==4)
 {
   sensor_countB[4]=sen5_counter/1.114;
   digitalWrite(sensor5_s2,LOW);
   digitalWrite(sensor5_s3,LOW);
 }
 else
 {
   sensor5_flag=0; 
   TIMSK2 = 0x00;
  }
  
sen5_counter=0; // reset counter
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

void pin_ISR() 
{  
  if(!timerActive)
  {
    startTime = millis();  
    lastButtonPush = startTime;
    timerActive = true;
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
        unlockDoor();
        timerActive = false; 
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
  
  long startTime = millis();
  long delayTime = 0;
  
  timerActive = false;
  Serial.println("RECALIBRATED!**************");
  recalibrated = true;
  return;
}
 
void printColor(int sensor, int color)
{
  
  Serial.print("*** Flag for sensor ");
  Serial.print(sensor);
  Serial.print(" detected looks ");
  switch (color) 
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
 
 void printRGB(int sensor)
 {
  Serial.print("Sensor ");
  Serial.print(sensor);
  Serial.print("(R,G,B)=(");
  Serial.print(sensor_countR[sensor-1],DEC); 
  Serial.print(",");
  Serial.print(sensor_countG[sensor-1],DEC); 
  Serial.print(",");
  Serial.print(sensor_countB[sensor-1],DEC); 
  Serial.println(")");
 }
 
 void lockDoor()
 {
   // Reset the solution
   sensorColors[0]=NONE;
   sensorColors[1]=NONE;
   sensorColors[2]=NONE;
   sensorColors[3]=NONE;
   sensorColors[4]=NONE;
   sensor_countR[0]=NONE;
   sensor_countR[1]=NONE;
   sensor_countR[2]=NONE;
   sensor_countR[3]=NONE;
   sensor_countR[4]=NONE;
   sensor_countG[0]=NONE;
   sensor_countG[1]=NONE;
   sensor_countG[2]=NONE;
   sensor_countG[3]=NONE;
   sensor_countG[4]=NONE;
   sensor_countB[0]=NONE;
   sensor_countB[1]=NONE;
   sensor_countB[2]=NONE;
   sensor_countB[3]=NONE;
   sensor_countB[4]=NONE;
  
   // Start timer for motio sensor
   timeDoorLocked = millis();
   
   // Lock the door by activating the relay
   digitalWrite(relayPin, HIGH);
   isDoorLocked = true;
   Serial.println("Locking door.");
 }
 
 void unlockDoor()
 {
   digitalWrite(relayPin, LOW);
   timerActive = false;
   isDoorLocked = false;
   Serial.println("Unlocking door.");
   deactivateMotionSensor();
 }

void getNumFromKeypad()
{
  char key = keypad.getKey();
  if(key)
  {
    Serial.println("NUMPAD before sequence is ");
    Serial.println(keypadSequence);

    keypadSequence.remove(0,1);
    keypadSequence+=key;
    Serial.println("NUMPAD sequence is ");
    Serial.println(keypadSequence);
    checkKeypad();
  } 
}

void checkKeypad()
{
  
  // First check to see if we're getting a recalibrate command
  if(!isDoorLocked)
  {
    // First check to see if recalibrate command has been given
    if(keypadSequence.endsWith("4983*#"))
    {      
      lockDoor();
      // Start the recalibration timer
      recalibrationTimer.after(recalibrationDelay, recalibrateSensors);
    }
    else if(keypadSequence.endsWith("4983#"))
    {
      lockDoor();
    }
    else if(keypadSequence.endsWith("4983*1#"))
    {
      MAX_COLOR_TOLERANCE-=3;
      if(MAX_COLOR_TOLERANCE < 1)
      {
        MAX_COLOR_TOLERANCE=3;
      }
      Serial.print("Changing MAX_COLOR_TOLERANCE to: ");
      Serial.println(MAX_COLOR_TOLERANCE);
      lockDoor();
    }
    else if(keypadSequence.endsWith("4983*9#"))
    {
      MAX_COLOR_TOLERANCE+=3;
      Serial.print("Changing MAX_COLOR_TOLERANCE to: ");
      Serial.println(MAX_COLOR_TOLERANCE);
      lockDoor();
    }
    
  } 
}

void checkMotionSensor()
{
  // Don't bother checking value if door isn't locked 
  if(isDoorLocked)
  {
    if(motionSensorActive)
    {
      motionVal=analogRead(motionSensorPin);
      if(DEBUG_MODE)
      {
        Serial.print("distance reading:");
        Serial.println(motionVal);
      }
      if(motionVal > motionThreshold)
      {
        unlockDoor(); 
        Serial.println("Unlocking door due to motion detected in locked room!");
      }
    }
    else
    {
      // Check to see if the wait window has expired
      if(millis() - timeDoorLocked > motionSensorDelayWindow)
      {
        motionSensorActive = true;
      } 
    }
  }
}

void deactivateMotionSensor()
{
  motionSensorActive=false; 
  Serial.println("Deactivating motion sensor."); 
}
 
 void loop()
 {
  motionSensorTimer.update(); 
  recalibrationTimer.update();

  getNumFromKeypad();
  
  if(isDoorLocked)
  {
    TCS();
    setColorForSensor(1, (int)sensor_countR[0], (int)sensor_countG[0], (int)sensor_countB[0]);
    setColorForSensor(2, (int)sensor_countR[1], (int)sensor_countG[1], (int)sensor_countB[1]);
    setColorForSensor(3, (int)sensor_countR[2], (int)sensor_countG[2], (int)sensor_countB[2]);
    setColorForSensor(4, (int)sensor_countR[3], (int)sensor_countG[3], (int)sensor_countB[3]);
    setColorForSensor(5, (int)sensor_countR[4], (int)sensor_countG[4], (int)sensor_countB[4]);
    if(!timerActive)
    {
      if(recalibrated)
      {
         delay(3000);
         recalibrated = false; 
      }
      
      if(!DEBUG_MODE)
      {
        printColor(1, sensorColors[0]);
        printRGB(1);
        printColor(2, sensorColors[1]);
        printRGB(2);
        printColor(3, sensorColors[2]);
        printRGB(3);
        printColor(4, sensorColors[3]);
        printRGB(4);
        printColor(5, sensorColors[4]);
        printRGB(5);
        Serial.println("");
       }
       delay(1000);  
       if(isCorrectSolution())
       {
         unlockDoor(); 
         printRGB(1);
         printRGB(2);
         printRGB(3);
         printRGB(4);
         printRGB(5);
       }
    }
  }  
 }
