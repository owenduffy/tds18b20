#define VERSION "0.03"
#include <Arduino.h>
#include <Wire.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
//#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define TEMPMIN -20
#define BARGRAPH
#define PPD 2 //pixels per degree, must be +ve integer
#define TICKMIN 5
#define TICKMAJ 10
//#define SSD1306_DISPLAY
//#define SH1106G_DISPLAY
#if defined(SSD1306_DISPLAY)
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display=Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif
#if defined(SH1106G_DISPLAY)
#include <Adafruit_SH110X.h>
#define WHITE SH110X_WHITE
#define BLACK SH110X_BLACK
Adafruit_SH1106G display=Adafruit_SH1106G(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire);
#endif
#include <DS18B20.h>
DS18B20 ds(2);

#if defined(__AVR_ATmega328P__)
HardwareSerial &MySerial=Serial;
#endif

int i;
int barh=SCREEN_HEIGHT/2-2;
int basey=display.height()-1;
int tickh=barh/4;

void setup(){
  float adcref;
  long adcfs;

  #if defined(__AVR_ATmega328P__)
  analogReference(INTERNAL);
  adcref=1.10;
  adcfs=1024;
  #endif
  analogRead(A2); //Read ADC2
  delay (500); // Allow ADC to settle
  float vbat=analogRead(A2); //Read ADC again
  vbat=16*(vbat + 0.5)/(float)adcfs*adcref; //Calculate battery voltage scaled by 150k & 10k
  // Display startup screen
  MySerial.begin(9600);
  MySerial.println(F("Starting..."));
  #if defined(SSD1306_DISPLAY)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //Initialize with the I2C address 0x3C.
  #endif
  #if defined(SH1106G_DISPLAY)
  display.begin(0x3C, true); // Address 0x3C default
  #endif
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("DS18B20 thermometer");
  display.setCursor(0, 12);
  display.print("ardds18b20 ver: ");
  display.println(VERSION);
  display.print("vbat: ");
  display.println(vbat,1);
  display.display();
  delay(1000);
}

void loop(){
  int i,j;
  float temp;
  uint8_t id[8];
  char buf[27];

  j=1;
  while (ds.selectNext()){
    //for each sensor
    ds.getAddress(id);
    sprintf(buf," %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X ",id[0],id[1],id[2],id[3],id[4],id[5],id[6],id[7]);
    temp=(ds.getTempC());
    MySerial.print(j);
    MySerial.print(buf);
    MySerial.print(temp,2);
    MySerial.println(F(" °"));
    display.clearDisplay();
    #if SCREEN_HEIGHT==32
    display.setCursor (0,0);
    display.setTextSize(2);
    display.print(j);
    display.print(F(": "));
    display.print(temp,1);
    display.print((char)247);
    #endif
    #if SCREEN_HEIGHT==64
    display.setCursor (0,5);
    display.setTextSize(3);
    display.print(j);
    display.print(F(":"));
    display.print(temp,1);
    #endif
    
    #if defined(BARGRAPH)
    int w=(temp-TEMPMIN)*PPD;
    //draw bar starting from left of screen:
    display.fillRect(0,display.height()-1-barh,w,barh,WHITE);
    display.fillRect(w+1,display.height()-barh-1,display.width()-w,barh,BLACK);
    //draw tick marks
    for(int i=0;i<SCREEN_WIDTH;i=i+PPD*TICKMIN)
      display.fillRect(i,basey-barh+3*tickh,1,barh-3*tickh,i>w?WHITE:BLACK);
    for(int i=0;i<SCREEN_WIDTH;i=i+PPD*TICKMAJ)
      display.fillRect(i,basey-barh+2*tickh,1,barh-2*tickh,i>w?WHITE:BLACK);
    if(TEMPMIN<0)
      display.fillRect((0-TEMPMIN)*PPD,basey-barh+tickh,1,barh-tickh,i>w?WHITE:BLACK);
    #endif

    display.display();
    j++;
    delay(1000);
    }
  delay(100);
  }
