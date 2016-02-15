/****** Preprocessor Macros *****/
#define RED 0
#define ORANGE 1
#define GREEN 2
#define PURPLE 3
#define BLUE 4

// MAX_COLOR_TOLERANCE is the maximum allowed devation between the expected and observed 
// values across the three color channels to consider a color to be successfully detected
#define MAX_COLOR_TOLERANCE 100

// These values are have been experimentally determined to be the average readings for the various flags
// For now, these are guesses
#define GREEN_FLAG_R 0
#define GREEN_FLAG_G 255
#define GREEN_FLAG_B 0

#define RED_FLAG_R 255
#define RED_FLAG_G 0
#define RED_FLAG_B 0

#define ORANGE_FLAG_R 255
#define ORANGE_FLAG_G 165
#define ORANGE_FLAG_B 0

#define PURPLE_FLAG_R 160
#define PURPLE_FLAG_G 32
#define PURPLE_FLAG_B 240

#define BLUE_FLAG_R 0
#define BLUE_FLAG_G 0
#define BLUE_FLAG_B 255

/***** GLOBAL VARIABLES *****/
// Color Sensor One (Looking for red)
int sensor1_s0=3;
int sensor1_s1=4;
int sensor1_s2=5;
int sensor1_s3=6;
int sensor1_out=7;

// Color Sensor Two (Looking for orange)
int sensor2_s0=22;
int sensor2_s1=24;
int sensor2_s2=26;
int sensor2_s3=28;
int sensor2_out=30;

// Color Sensor Three (Looking for green)
int sensor3_s0=32;
int sensor3_s1=34;
int sensor3_s2=36;
int sensor3_s3=38;
int sensor3_out=40;

// Color Sensor Four (Looking for purple)
int sensor4_s0=42;
int sensor4_s1=44;
int sensor4_s2=46;
int sensor4_s3=48;
int sensor4_out=50;

// Color Sensor Five (Looking for blue)
int sensor5_s0=23;
int sensor5_s1=25;
int sensor5_s2=27;
int sensor5_s3=29;
int sensor5_out=31;

//int s0=3,s1=4,s2=5,s3=6;
//int out=2;
int flag=0;
byte sen1_counter=0;
byte sen1_countR=0,sen1_countG=0,sen1_countB=0;
void setup()
 {
 // Open serial connection with console
 Serial.begin(115200);
 
 // Declare sensor pins as outputs
 //pinMode(s0,OUTPUT);
 //pinMode(s1,OUTPUT); 
 //pinMode(s2,OUTPUT);
 //pinMode(s3,OUTPUT);
 // Sensor 1
 pinMode(sensor1_s0,OUTPUT);
 pinMode(sensor1_s1,OUTPUT); 
 pinMode(sensor1_s2,OUTPUT);
 pinMode(sensor1_s3,OUTPUT);
 // Sensor 2
 pinMode(sensor2_s0,OUTPUT);
 pinMode(sensor2_s1,OUTPUT); 
 pinMode(sensor2_s2,OUTPUT);
 pinMode(sensor2_s3,OUTPUT);
 // Sensor 3
 pinMode(sensor3_s0,OUTPUT);
 pinMode(sensor3_s1,OUTPUT); 
 pinMode(sensor3_s2,OUTPUT);
 pinMode(sensor3_s3,OUTPUT);
 // Sensor 4
 pinMode(sensor4_s0,OUTPUT);
 pinMode(sensor4_s1,OUTPUT); 
 pinMode(sensor4_s2,OUTPUT);
 pinMode(sensor4_s3,OUTPUT);
 // Sensor 5
 pinMode(sensor5_s0,OUTPUT);
 pinMode(sensor5_s1,OUTPUT); 
 pinMode(sensor5_s2,OUTPUT);
 pinMode(sensor5_s3,OUTPUT);

 }
void TCS()
 {
 flag=0;  
 digitalWrite(sensor1_s1,HIGH);
 digitalWrite(sensor1_s0,HIGH);
 digitalWrite(sensor1_s2,LOW);
 digitalWrite(sensor1_s3,LOW);
 attachInterrupt(digitalPinToInterrupt(sensor1_out), ISR_INTO, LOW);
 
 // IMPORTANT - On the Mega, valid interrupt pins are 2, 3, 18, 19, 20, 21. 
 // We'll need to remap all of the 'sensorX_out' pins to one of these options.
 // It would appear the original programmers took the unrecommended shortcut
 // of simply assuming that interrupt 0 corresponds to pin 2 on a stock arduino
 timer0_init();

 }
void ISR_INTO()
 {
 sen1_counter++;
 }
 void timer0_init(void)
 {
  TCCR2A=0x00;
  TCCR2B=0x07;   //the clock frequency source 1024 points
  TCNT2= 100;    //10 ms overflow again
  TIMSK2 = 0x01; //allow interrupt
 }
 int i=0;
 ISR(TIMER2_OVF_vect)//the timer 2, 10ms interrupt overflow again. Internal overflow interrupt executive function
{
    TCNT2=100;
    flag++;
 if(flag==1)
  {
    sen1_countR=sen1_counter;
    Serial.print("red=");
    Serial.println(sen1_countR,DEC);
    digitalWrite(sensor1_s2,HIGH);
    digitalWrite(sensor1_s3,HIGH);
  }
  else if(flag==2)
   {
    sen1_countG=sen1_counter;
    Serial.print("green=");
    Serial.println(sen1_countG,DEC);
    digitalWrite(sensor1_s2,LOW);
    digitalWrite(sensor1_s3,HIGH);
   }
   else if(flag==3)
    {
    sen1_countB=sen1_counter;
    Serial.print("blue=");
    Serial.println(sen1_countB,DEC);
    Serial.println("\n"); 
    digitalWrite(sensor1_s2,LOW);
    digitalWrite(sensor1_s3,LOW);
   
    }
    else if(flag==4)
     {
       Serial.print("Found Red flag is :");
       if(isColor(RED, sen1_countR, sen1_countG, sen1_countB))
       {
         Serial.println("\n\n!!!RED FLAG DETECTED IN FRONT OF SENSOR!!!\n\n");
       }
       {
         Serial.println("Red flag not detected.\n");
       }
     flag=0;
     }
       sen1_counter=0;
}

boolean isColor(int COLOR, byte red, byte green, byte blue)
{
   // This function takes in a color and returns whether or not the
   // sensor is seeing that color
   
   int absDiff = 0; // variable to hold the absolute difference between the expected value of the color and the observed
   
   
   switch (COLOR) 
   {
    case RED:
      absDiff = abs( ((int)red - RED_FLAG_R) + ((int)green - RED_FLAG_G) + ((int)blue - RED_FLAG_B) );
      break;
    case ORANGE:
      absDiff = abs( ((int)red - ORANGE_FLAG_R) + ((int)green - ORANGE_FLAG_G) + ((int)blue - ORANGE_FLAG_B) );
      break;
    case BLUE:
      absDiff = abs( ((int)red - BLUE_FLAG_R) + ((int)green - BLUE_FLAG_G) + ((int)blue - BLUE_FLAG_B) );
      break;
    case PURPLE:
      absDiff = abs( ((int)red - PURPLE_FLAG_R) + ((int)green - PURPLE_FLAG_G) + ((int)blue - PURPLE_FLAG_B) );
      break;
    case GREEN:
      absDiff = abs( ((int)red - GREEN_FLAG_R) + ((int)green - GREEN_FLAG_G) + ((int)blue - GREEN_FLAG_B) );
      break;
    }
    
  if (absDiff < MAX_COLOR_TOLERANCE)
  {
    return true;
  }
  else
  {
    return false;
  }  
}
void loop()
 {
  TCS();
while(1);
 }
