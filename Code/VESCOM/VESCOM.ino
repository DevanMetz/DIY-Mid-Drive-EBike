#include <FS.h>
#include <FastLED.h>
#include <SPI.h>
#include <TFT_eSPI.h>       // Hardware-specific library
#include <Wire.h>
#include <VescUart.h>
#define TFT_GREY 0x5AEB
#define LED_PIN     33
#define NUM_LEDS    11
#define BRIGHTNESS  100
CRGB leds[NUM_LEDS];


VescUart UART;

bool ledFlash = true;
int rpm;
float voltage;
float current;
int power;
float amphour;
float tach;
float distance;
float velocity;
float watthour;
float batpercentage;
int xpos = 0;
int ypos = 0;

CHSV colorStart = CHSV(96,255,255);  // starting color
CHSV colorTarget = CHSV(192,255,255);  // target color
CHSV colorCurrent = colorStart;

TFT_eSPI tft = TFT_eSPI();

void setup(void) {

  Serial.begin(250000);
  Serial2.begin(115200, SERIAL_8N1, 25, 26);
  UART.setSerialPort(&Serial2);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  tft.begin();
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN,TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.drawString(String("Bat: "),0,0,4);
  tft.drawString(String("Watts: "),100,0,4);

  tft.setTextSize(2);
  tft.drawString(String("MPH: "),0,25,4);
  
  
}

void loop() {

  getValues();
  tft.fillRect(170, 0, 70, 20, TFT_BLACK);
  tft.fillRect(190, 25, 40, 40, TFT_BLACK);
  
  xpos = 50;
  ypos = 0;
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN,TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  String batString = String(voltage,1);
  tft.drawString(batString, xpos, ypos, 4);
  
  xpos = 170;
  ypos = 0;
  tft.setTextDatum(TL_DATUM);
  String powerString = String(power);
  tft.drawString(powerString, xpos, ypos, 4);

  xpos = 0;
  ypos = 110;
  tft.setTextDatum(TL_DATUM);
  String distanceString = String("Miles: " + String(distance));
  tft.drawString(distanceString, xpos, ypos, 4);

  xpos = 130;
  ypos = 110;
  tft.setTextDatum(TL_DATUM);
  String ampString = String("Ah: " + String(amphour));
  tft.drawString(ampString, xpos, ypos, 4);
  

  
  xpos = 120;
  ypos = 25;
  tft.setTextSize(2);
  tft.setTextDatum(TL_DATUM);
  String speedString = String(velocity,1);
  tft.drawString(speedString, xpos, ypos, 4);


    static uint8_t k;  // the amount to blend [0-255]
    if ( colorCurrent.h == colorTarget.h ) {  // Check if target has been reached
      colorStart = colorCurrent;
      colorTarget = CHSV(random8(),255,255);  // new target to transition toward
      k = 0;  // reset k value
      Serial.print("New colorTarget:\t\t\t"); Serial.println(colorTarget.h);
    }

    colorCurrent = blend(colorStart, colorTarget, k, SHORTEST_HUES);
    fill_solid( leds, NUM_LEDS, colorCurrent );
    leds[0] = colorTarget;  // set first pixel to always show target color
    Serial.print("colorCurrent:\t"); Serial.print(colorCurrent.h); Serial.print("\t");
    Serial.print("colorTarget:\t"); Serial.print(colorTarget.h);
    Serial.print("\tk: "); Serial.println(k);
    k++;
    
  FastLED.show();  // update the display
}

// Draw a + mark centred on x,y
void drawDatumMarker(int x, int y)
{
  tft.drawLine(x - 5, y, x + 5, y, TFT_GREEN);
  tft.drawLine(x, y - 5, x, y + 5, TFT_GREEN);
}
void getValues(){
  if ( UART.getVescValues() ) {
    rpm = (UART.data.rpm)/12;          // The '7' is the number of pole pairs in the motor. Most motors have 14 poles, therefore 7 pole pairs
    voltage = (UART.data.inpVoltage);
    current = (UART.data.avgInputCurrent);
    power = voltage*current;
    amphour = (UART.data.ampHours);
    watthour = amphour*voltage;
    tach = (UART.data.tachometerAbs)/72;          // The '42' is the number of motor poles multiplied by 3
    distance = tach*3.142*0.71/14481 ;          // Motor RPM x Pi x (1 / meters in a mile or km) x Wheel diameter x (motor pulley / wheelpulley)
    velocity = rpm*3.142*60*0.71/14481;          // Motor RPM x Pi x (seconds in a minute / meters in a mile) x Wheel diameter x (motor pulley / wheelpulley)
  }
}
