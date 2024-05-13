//Authors: Hannah Haggerty and Alyssa Aragon
//CPE301 Final Project May 12, 2024


#include <RTClib.h>
#include <string.h>
#include <Stepper.h>
#include <LiquidCrystal.h>
#include <DHT.h>

//DHT pin num and DHT type
#define DHT_PIN 7
#define DHTTYPE DHT11
#define FAN  0x10

//LCD length and pin num
#define LENGTHlcd 16
LiquidCrystal lcd(12, 11, 6, 5, 4, 3);

//Buttons
#define START 0x08
#define RESET 0x04
#define CONTROL 0x02

//LED light pin num
#define GREEN 0x80
#define YELLOW 0x20
#define RED 0x08
#define BLUE 0x02


#define RBE 0x80
#define TBE 0x20

#define PCI  0x0C    

//Buttons
volatile unsigned char *PORT_B  = (unsigned char *) 0x25;
volatile unsigned char *DDR_B   = (unsigned char *) 0x24;
volatile unsigned char *pinB   = (unsigned char *) 0x23;

//Water sensor
volatile unsigned char* thisADMUX = (unsigned char*) 0x7C;
volatile unsigned char* thisADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* thisADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* thisADCDATA = (unsigned int*) 0x78;

//the LED lights
volatile unsigned char *PORT_C  = (unsigned char *) 0x28;
volatile unsigned char *DDR_C   = (unsigned char *) 0x27;
volatile unsigned char *PIN_C   = (unsigned char *) 0x26;


//Print to the Serial monitor
volatile unsigned char *uscrC0 = (unsigned char *) 0x00C2;
volatile unsigned char *uscr0A = (unsigned char *) 0x00C0;
volatile unsigned char *uscrB0 = (unsigned char *) 0x00C1;
volatile unsigned char *ubrr0  = (unsigned char *) 0x00C4;
volatile unsigned char *zerudr   = (unsigned char *) 0x00C6;
volatile unsigned char *pcicrLocal   = (unsigned char *) 0x68;
volatile unsigned char *localpcifr   = (unsigned char *) 0x3B;

// Registers for Pin Change Interrupts
volatile unsigned char *thisPCICR   = (unsigned char *) 0x68;
volatile unsigned char *thisPCIFR   = (unsigned char *) 0x3B;
volatile unsigned char *thisPCMSK0  = (unsigned char *) 0x6B;


char lcd_buf[LENGTHlcd], err_msg[LENGTHlcd];
char mapsstate[4][16] = {"(DISABLED)", "IDLE", "ERROR", "RUNNING"};
unsigned char led_masks[4] = {YELLOW, GREEN, RED, BLUE};

//define the different running states
typedef enum STATES{
  DISABLED, 
  IDLE, 
  ERROR, 
  RUNNING} STATES;

STATES stateNow = DISABLED;

DHT dht(DHT_PIN, DHTTYPE);
RTC_DS3231 rtc;
Stepper step = Stepper(2038, 28, 26, 24, 22);

STATES oldState;

const unsigned int minTemp = 30, minWater = 40;
unsigned int waterLevel = 0, timerUpdates = 0;

void setup(){
  //set up all the components to begin
  lcd.begin(16, 2);

  //delay so the dht can begin after
  delay(2000);
  dht.begin();
  rtc.begin();
  rtc.adjust( DateTime (F(__DATE__), F(__TIME__)));
  lcd.clear();
  step.setSpeed(2);
  IO_INIT();
  initializeAdc();

  initalizeUart(9600); //baud rate i think
}

void loop(){
  waterLevel = readADC(0);
  DateTime now =rtc.now( );
  if (now.second()== 0){
    getTempHumid(lcd_buf);
  }

  lcd.setCursor(0, 1);
  lcd.print(mapsstate[stateNow]);

  updateLights(stateNow);
  switch (stateNow) { //switch between the differnet states of the cooler
    case DISABLED: //YELLOw
      *PORT_B &= ~FAN;
      break;

    case IDLE: //GREEN
      *PORT_B &= ~FAN;
      lcd.setCursor(0, 0);
      lcd.print(lcd_buf);
      if ((int)dht.readTemperature(true) >= minTemp) {
        stateNow = RUNNING;
      }
      if (waterLevel < minWater) {
        snprintf(err_msg, LENGTHlcd, "Low water");
        stateNow = ERROR;
      }
      break;

    case ERROR: //RED
      *PORT_B &= ~FAN;

      lcd.setCursor(0, 0);

      lcd.print(err_msg);
      break;

    case RUNNING: //blue and fan should be on
      lcd.setCursor(0, 0);
      lcd.print(lcd_buf);
      *PORT_B |= FAN;
      if ((unsigned int)dht.readTemperature(true) < minTemp) {
        stateNow = IDLE;
        *PORT_C &= ~BLUE;
      }
      if (waterLevel<minWater) {
        snprintf(err_msg, LENGTHlcd, "Low water!");
        stateNow = ERROR;
        *PORT_C &= ~BLUE;
      }
      break;
  }

  if (*pinB & CONTROL) {
    unsigned char bufferS[64];
    snprintf(bufferS, 64, "\vent updated.\n");
    uartString(bufferS, strlen(bufferS));
    step.step(1);
  }

  if (oldState != stateNow) {
    unsigned char bufferS[256];
    snprintf(bufferS, 256, "\nSTATE TRANSISTION: %s -> %s", mapsstate[oldState], mapsstate[stateNow]);
    uartString(bufferS, strlen(bufferS));
    snprintf(bufferS, 256, "\nCurrent Time: %02d:%02d:%02d\nCurrent Date: %d/%d/%d\n", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());
    uartString(bufferS, strlen(bufferS));
    if (stateNow == IDLE || stateNow == RUNNING) getTempHumid(lcd_buf);
    lcd.clear();
  }

  oldState = stateNow;
}void IO_INIT( ){
  *DDR_C |= GREEN;
  *DDR_C |= YELLOW; //initialize input/output
  *DDR_C |= RED;
  *DDR_C |= BLUE;
  *DDR_B |= FAN;
  *PORT_B |= START;
  *DDR_B &= ~(START);

  *PORT_B |= RESET;
  *DDR_B &= ~(RESET);

  *PORT_B |= CONTROL;
  *DDR_B  &= ~(CONTROL);
  *pcicrLocal |= 0x01;
  *thisPCMSK0 |= PCI;
}

//initialize ADC
void initializeAdc() {
  *thisADCSRA |=  0b10000000; 
  *thisADCSRA &=  0b11011111; 
  *thisADCSRB &=  0b11110111; 
  *thisADCSRB &=  0b11111000; 
  *thisADMUX &= 0b01111111; 
  *thisADMUX |= 0b01000000; 
  *thisADMUX &= 0b11011111; 
  *thisADMUX &= 0b11100000;
}


          

void updateLights(STATES state){
  *PORT_C = led_masks[state]; //led
}
//initialize UART
void initalizeUart(unsigned long baud) {
  unsigned long FCPU = 16000000;
  unsigned int tbaud = (FCPU/16/baud - 1);
  *uscr0A = 0x20;
  *uscrB0 = 0x18;
  *uscrC0 = 0x06;
  *ubrr0  = tbaud;
}


ISR(PCINT0_vect) {//ISR for pin change interrupt 0
  if (*pinB & RESET){
    if (stateNow == ERROR) {
      stateNow = IDLE;
    }
  } else if (*pinB & START) {
    if (stateNow == RUNNING || stateNow == IDLE || stateNow == ERROR) {
      stateNow = DISABLED;
    } else if (stateNow == DISABLED) {
      stateNow = IDLE;
    }
  }
}


unsigned int readADC(unsigned char adcNum) {
  *thisADMUX &= 0b11100000;
  *thisADCSRB &= 0b11110111;
  
  if(adcNum > 7) {
    adcNum -= 8;
    *thisADCSRB |= 0b00001000;
  }
  
  *thisADMUX += adcNum;
  *thisADCSRA |= 0x40;
  
  while((*thisADCSRA & 0x40) != 0);
  
  return *thisADCDATA;}
//sends a single character via UART
void uartCharacter(unsigned char character){
  while ((*uscr0A & TBE) == 0) { };
  *zerudr = character;
}
//load humidity and temperature values into character buffer for LCD display
void getTempHumid(char *buffer) {
  int humidity = (int)dht.readHumidity();
  int temp = (int)dht.readTemperature(true);

  snprintf(buffer, LENGTHlcd, "Humidity: %d Temperature:%dF", humidity, temp);
}

//sends a string of a given length via theUART
void uartString(unsigned char *string, int lengthString){
  while ((*uscr0A & TBE) == 0) { };

  for (int i=0; i<lengthString && string[i] != '\0'; i++) {
    while ((*uscr0A & TBE) == 0) {};
    *zerudr = string[i];
   }

}
