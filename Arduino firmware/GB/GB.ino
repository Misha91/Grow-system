


#include "DHT.h"
#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF

#define __CS 10
#define __DC 9
#define __RES 12

TFT_ILI9163C display = TFT_ILI9163C(__CS, __DC, __RES);

RTC_DS1307 RTC;

#define DHTPIN 7     // what digital pin we're connected to
#define buttonPin 8
#define lightPin 2
#define fanPin 3
#define tempPin 3

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);
  
  

void tempCheck(void);
void timeCheck(void);
void buttonCheck(void);
void routine(void);
void lcdUpdate(void);
void fanOn(void);
void fanOff(void);
int lastSecond = 0;
int Pressed = 0;
int Pressed_Confidence_Level = 0; //Measure button press cofidence
int Released_Confidence_Level = 0; //Measure button release confidence
int highTemp=0;
int counter;
int ny, nM, nd, nh, nm, ns;
int lightStatus(0), fanStatus(0);
float t, h, f, tOut;


void setup(void)
{
  
  dht.begin();
  Serial.begin(9600);
 

  Wire.begin();
  //RTC.begin();
  pinMode(buttonPin, INPUT);
  pinMode(lightPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
 


  display.begin();
  display.setRotation(0);
  display.fillScreen(BLACK);
  
  display.setTextWrap(true);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0,0);

 
}

void loop(void)
{
  lcdUpdate();
  
  everySecond();
  buttonCheck();
  delay(100);
}



void tempCheck(void){
int interm;
  
h = dht.readHumidity();
  // Read temperature as Celsius (the default)
t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    //Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

      for (int i(0); i<20; i++){
     
      interm += analogRead(tempPin);

      if (i == 19){
      float tBuff = (interm / 20);
      tOut = -7*0.00001*tBuff*tBuff - 0.0019*tBuff + 62.255;
      interm = 0;
      }
      
      
    }
      
   
  
}



void everySecond(void){
  DateTime now = RTC.now();
  nh = now.hour();
  nm = now.minute();
  ns = now.second();
  nd = now.day();
  nM = now.month();
  ny = now.year();

if (ns != lastSecond){
    tempCheck();
    routine();
    lastSecond = ns;
}
}

void buttonCheck(void){
if (digitalRead(buttonPin)){
   Pressed_Confidence_Level ++; //Increase Pressed Conficence
 Released_Confidence_Level = 0; //Reset released button confidence since there is a button press
    
    if (Pressed_Confidence_Level >10) //Indicator of good button press
    {
      if (Pressed == 0)
      {
      display.clearScreen();
      display.setCursor(0,0);
      display.println("Restarting...");
      digitalWrite(lightPin, LOW);
      digitalWrite(fanPin, LOW);
      lightStatus = 0;
      fanStatus = 0;
      delay(2500);
      RTC.adjust(DateTime(2001,1,1,0,0,0));
      }
      
      Pressed_Confidence_Level = 0;

    }
  }
//Zero it so a new pressed condition can be evaluated


else
  {
  Released_Confidence_Level ++; //This works just like the pressed
  Pressed_Confidence_Level = 0; //Reset pressed button confidence since the button is released
    if (Released_Confidence_Level >10)
    {
    Pressed = 0;
    Released_Confidence_Level = 0;
    } 
  }
    
   
}

void routine(void){



  if (nh >= 0 && nh <= 15){
    if (lightStatus == 0){
        digitalWrite(lightPin, HIGH);
        delay(100);
        lightStatus = 1;
       // gotoXY(0,3);
       // LcdString("L ON ");
    }
  }
  else if (nh > 15){
    if (lightStatus){
        digitalWrite(lightPin, LOW);
        delay(100);
        lightStatus = 0;
     //   gotoXY(0,3);
     //   LcdString("L OFF");
         }
  }

  if ( ((nm >= 1 && nm <= 4) ||  (nm >= 31 && nm <= 34)) || highTemp == 1){
   fanOn();
  }

    else if ( ( !(nm >= 1 && nm <= 4) && !(nm >= 16 && nm <= 18) && !(nm >= 31 && nm <= 34) && !(nm >= 46 && nm <= 51)) && highTemp == 0){
    fanOff();
  }

  if (highTemp == 0){
  if ( t >= (tOut + 2) && tOut <= 26) 
  {
       
    if (nm < 57){
    counter = nm +3;
    highTemp = 1;
    }

    if (nm >= 57){
    counter = nm + 3 - 60;
    highTemp = 1;
    }
  }
  }
  
   
    if (highTemp == 1){
    if (nm == counter) {
      highTemp = 0;
    }   
  }
  
}

void lcdUpdate(void){
 // display.clearScreen();
  //
 
  display.setCursor(0,0);
  display.println("Current time:");

  char buf[10];
  char buf2[15];
  
  sprintf(buf,"%02d:%02d:%02d", nh, nm, ns );
 // display.setTextSize(1);
 // display.setCursor(0,1);
  display.println(buf);
  
  //преобр в строку число месяц год
  sprintf(buf2,"D%02d M%02d", nd, nM);
 // display.setCursor(0,2);
  display.println(buf2);
 // display.fillScreen();

if (fanStatus == 1){
    if (lightStatus == 1) display.println("L: ON, F: ON   ");
    else display.println("L: OFF, F: ON   ");
  }

if (fanStatus == 0)
  {
   if (lightStatus == 1) display.println("L: ON, F: OFF ");
    else display.println("L: OFF, F: OFF");
    
  }
 char buf3[100];
     int d1 = t;            // Get the integer part (678).
     float f2 = t - d1;     // Get fractional part (678.0123 - 678 = 0.0123).
    int d2 = trunc(f2 * 10);   // Turn into integer (123).
    int d3 = h;            // Get the integer part (678).
     float f3 = h - d1;     // Get fractional part (678.0123 - 678 = 0.0123).
    int d4 = trunc(f2 * 10);   // Turn into integer (123).
  sprintf(buf3, "%02d.%dC %02d.%dRH", d1, d2, d3, d4);
  int a = t;
   display.println(buf3);

  d1 = tOut;
  f2 = tOut - d1;
  d2 = trunc(f2 * 10);   // Turn into integer (123).
  sprintf(buf3, "T out %02d.%dC ", d1, d2);
  display.println(buf3);
 
  
}

void fanOn(void){
   if (fanStatus == 0){
        digitalWrite(fanPin, HIGH);
        delay(100);
        fanStatus = 1;
    }
}

void fanOff(void){
  if (fanStatus){
        digitalWrite(fanPin, LOW);\
        delay(100);
        fanStatus = 0;

         }

}


