
/* Digital clock with alarm system
 *  The circuit schematic is inspired by Richardo Moreno's schematic for his 4 digit 7 segment display setup.
 *  The DisplaySegments() function is based on code from Richardo Moreno's (rmorenojr on Github) 4 digit 7 segment display setup (lesson 28).
    https://github.com/rmorenojr/ElegooTutorial
 */

#include <pitches.h>
#include <RTClib.h>
#include <Wire.h> //allows device to communicate via I2C protocol

RTC_DS3231 rtc; //rtc object
DateTime now;

int melody_part1[] = {  //notes to play twinkle twinkle littler start
  NOTE_C5, NOTE_C5, NOTE_G5, NOTE_G5, NOTE_A5, NOTE_A5, NOTE_G5, NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5, NOTE_D5, NOTE_C5};

int melody_part2[] = {
  NOTE_G5, NOTE_G5, NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5};

const int dataPin  = 9;  // 74HC595 pin 8 DS
const int latchPin = 11;  // 74HC595 pin 9 STCP
const int clockPin = 12;   // 74HC595 pin 10 SHCP
const int digit1   = 8;   // First digit
const int digit2   = 7;   // Second digit
const int digit3   = 6;   // Third digit
const int digit4   = 5;   // Fourth digit

int num1;
int num2;
int num3; 
int num4;

const int note = 440; //middle A frequency
const int buzzerPin = 4;    // For alarm
const int hourButton = 3;   // Adjust alarm time by adding hours
const int minuteButton = 2; // Adjust alarm time by adding minutes

int alarmTime[] = {0,6,0,0}; //default time is 6 am

byte table[]=   //hexadecimal values for displaying numbers on display
    {   0x5F,  // = 0
        0x44,  // = 1
        0x9D,  // = 2
        0xD5,  // = 3
        0xC6,  // = 4
        0xD3,  // = 5
        0xDB,  // = 6
        0x45,  // = 7
        0xDF,  // = 8
        0xD7,  // = 9
        //0x00   // blank 
    };

//byte digitDP = 0x20;  // adds this to digit to show decimal point
byte digits[] = {digit1, digit2, digit3, digit4};

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  rtc.begin();
  
  if (! rtc.begin())  //if connection fails
  {
    Serial.println(" RTC Module not Present");
    while (1);
  }

  if (rtc.lostPower())  //if power is momentarily lost
  {
    Serial.println("RTC power failure, reset the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));  //get current date and time
  
  pinMode(latchPin,OUTPUT); //shift register pins will output data
  pinMode(clockPin,OUTPUT);
  pinMode(dataPin,OUTPUT);

  pinMode(hourButton, INPUT_PULLUP);  //default value is high 
  pinMode(minuteButton, INPUT_PULLUP);

  for (int x=0; x<4; x++){
     pinMode(digits[x],OUTPUT);  //digit pins output data
     digitalWrite(digits[x],LOW);  // Turns off the digit  
  }
}

void DisplaySegments(int num1, int num2, int num3, int num4){ // Sends out 4 numbers (the input arguments) to the display
    int numArray[] = {num1, num2, num3, num4};  //the four numbers to be displayed
    
    for (int i=0; i<4; i++){  //for each of the four digits
        for (int j=0; j<4; j++){
            digitalWrite(digits[j],LOW);    // turn off all four digits
        }
        digitalWrite(latchPin,LOW);
        shiftOut(dataPin,clockPin,MSBFIRST,table[numArray[i]]); //send bits for corresponding digit
        digitalWrite(latchPin,HIGH);
        
        digitalWrite(digits[i],HIGH);   // turn on one digit
        delay(2);                              // 1 or 2 is ok
    }
    for (int j=0; j<4; j++){
        digitalWrite(digits[j],LOW);    // turn off all four digits
    }
}


void AdjustAlarmHour(){ //increase alarm time by 1 hour
   Serial.print("adding 1 hour to alarm time\n");

   int alarm_hour = alarmTime[0] * 10 + alarmTime[1];
   
   if(alarm_hour == 23){
      alarm_hour = 0;
   }
   else{
     alarm_hour++; 
   }

   alarmTime[0] = alarm_hour / 10;
   alarmTime[1] = alarm_hour % 10;
   
   for(int i = 0; i < 50; i++){     //display new alarm time
    DisplaySegments(alarmTime[0], alarmTime[1], alarmTime[2], alarmTime[3]);
  }
}

void AdjustAlarmMinute(){ //increase alarm time by 1 minute
   Serial.print("adding 1 minute to alarm time\n");
 
   int alarm_minute = alarmTime[2] * 10 + alarmTime[3];

   if(alarm_minute == 59){
    alarm_minute = 0;
    AdjustAlarmHour();
   }
   else{
    alarm_minute++;
   }

   alarmTime[2] = alarm_minute / 10;
   alarmTime[3] = alarm_minute % 10;
   
   for(int i = 0; i < 50; i++){     //display new alarm time
    DisplaySegments(alarmTime[0], alarmTime[1], alarmTime[2], alarmTime[3]);
  }
}

void resetAlarm(){  //reset alarm time to 6am
   Serial.print("resetting alarm\n");
   alarmTime[0] = 0;
   alarmTime[1] = 6;
   alarmTime[2] = 0;
   alarmTime[3] = 0;

   for(int i = 0; i < 100; i++){     //display new alarm time
    DisplaySegments(alarmTime[0], alarmTime[1], alarmTime[2], alarmTime[3]);
  }
}

void alarmSound(){  //play a simple melody in a loop 
  if(digitalRead(minuteButton) == LOW && digitalRead(hourButton) == LOW){ //pressing both buttons while the alarm plays acts as a snooze button
    for(int i = 0; i < 5; i++){
      AdjustAlarmMinute();  //delays alarm by 5 minutes
    }
  }


  for(int i = 0; i < 14; i++){
    tone(buzzerPin, melody_part1[i], 500);
  }
  
  /*
  for(int i = 0; i < 13; i++){  //plays twinkle twinlke little star
    tone(buzzerPin, melody_part1[i], 500);
  }
  tone(buzzerPin, melody_part1[13], 1000);
  
  for(int i = 0; i < 2; i++){
    for(int j = 0; j < 6; j++){
      tone(buzzerPin,melody_part2[j], 500);
    }
    tone(buzzerPin,melody_part2[6], 1000);
  }
  for(int i = 0; i < 13; i++){
    tone(buzzerPin, melody_part1[i], 500);
  }
  tone(buzzerPin, melody_part1[13], 1000);*/
  
}

void loop() {
  // put your main code here, to run repeatedly:
  DateTime now = rtc.now();
  int cur_hour = now.hour();
  int cur_minute = now.minute();

  Serial.print(cur_hour);
  Serial.print(":");
  Serial.print(cur_minute);
  Serial.print("\n");
 

  num1 = cur_hour / 10; //first hour digit
  num2 = cur_hour % 10; //second hour digit

  num3 = cur_minute / 10; //first minute digit
  num4 = cur_minute % 10; //second minute digit

  if(digitalRead(minuteButton) == LOW && digitalRead(hourButton) == LOW){ //both buttons are pressed, reset alarm to default time (6am)
    resetAlarm();
  }

  if(digitalRead(hourButton) == LOW && digitalRead(minuteButton) == HIGH){ //hour button is presseed
    AdjustAlarmHour();
  }
  
  if(digitalRead(minuteButton) == LOW && digitalRead(hourButton) == HIGH){ //minute button is presseed
    AdjustAlarmMinute();
  }
  if(alarmTime[0] == num1 && alarmTime[1] == num2 &&
  alarmTime[2] == num3 && alarmTime[3] == num4){
    //tone(buzzerPin, note, 3000); //sound alarm, note played is middle A for 3 seconds
    alarmSound();
  }
  if(digitalRead(minuteButton) == HIGH && digitalRead(hourButton) == HIGH){ //no button is pressed, display current time;
    for(int i = 0; i < 100; i++){
      DisplaySegments(num1, num2, num3, num4);  
    }
  }
  
  delay(200); //allows clock to blink once per second
}
