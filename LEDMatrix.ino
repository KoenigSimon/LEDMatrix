#include <FastLED.h>
#include <LEDMatrix.h> 

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#define LED_PIN     23
#define NUM_LEDS    384
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define MOSFET 33
#define BUTTON 39
#define SCL 25
#define SDA 21

long temp[64] = {};
long result[64] = {};
long input[64] = {};
int currentprof = 0;

#include "DataStore.h"

static StaticJsonDocument<8192> doc;
DeserializationError error;
bool newDataReady = false;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long connectionTimer = 0;

float brightnessFactor = 1;

CRGB StatusLed[1];

void setup() {
  // put your setup code here, to run once:
  //turn off leds
  Serial.begin(115200);
  pinMode(MOSFET, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  digitalWrite(MOSFET, LOW);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  FastLED.addLeds<LED_TYPE, 27, COLOR_ORDER>(StatusLed, 1).setCorrection( TypicalLEDStrip );

  digitalWrite(MOSFET, LOW);

  Serial.println("WiFi Setup");
  wifi_setup();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.setBufferSize(8192);
  Serial.println("WiFi Setup finished");

  digitalWrite(MOSFET, HIGH);
}

void prof1()
{
  MirrorMatrix(left1, 8);
  fillPanel(2, result);

  MirrorMatrix(left2, 8);
  fillPanel(1, result);

  MirrorMatrix(left3, 8);
  fillPanel(0, result);
  

  //MirrorMatrix(left1, 8);
  fillPanel(3, left1);

  //MirrorMatrix(left2, 8);
  fillPanel(4, left2);

  //MirrorMatrix(left3, 8);
  fillPanel(5, left3);
}

void showProf()
{
  switch(currentprof)
  {
    case 0:
      prof1();
      break;  
    default:
      currentprof = 0;
      showProf();
      break;
  }
}

void loop() {
  if(!digitalRead(BUTTON)){
    while(!digitalRead(BUTTON));
    currentprof++;
  }  

  if (millis() > connectionTimer + 500)
  {
    connectionTimer = millis();
    if (!client.connected())
      reconnect();

    client.loop();
  }

  //showProf();

  if (newDataReady)
  {
    newDataReady = false;
    ReadData();
  }
  
  FastLED.show();
}

void ReadData()
{
  if (doc.containsKey("brightnessFactor"))
  {
    brightnessFactor = doc["brightnessFactor"] | 1.0;
  }
  
  for(int p = 0; p < 6; p++)
  {
    JsonObject panel = doc["panels"][p];
    if (panel.isNull()) continue;

    //write current panel
    for(int i = 0; i < 64; i++)
    {
      input[i] = strtol(panel["dots"][i], 0, 16);
    }    
    
    if (panel.containsKey("rot"))
    {
      for(int j = 0; j < (panel["rot"] | 0); j++)
      {
        if(j == 0) RotateMatrix(input, 8);
        else RotateMatrix(result, 8);
      }
    }
    
    if (panel.containsKey("mirr"))
    {
      if(panel["mirr"] == true)
      {
        if(panel.containsKey("rot") && panel["rot"] > 0) MirrorMatrix(result, 8);
        else MirrorMatrix(input, 8);
      }
      
    }    

    if(panel.containsKey("zigzag") && panel["zigzag"] == true)
    {
      if((panel.containsKey("mirr") && panel["mirr"] == true) || (panel.containsKey("rot") && panel["rot"] > 0)) fillPanelZigZag(p, result);
      else fillPanelZigZag(p, input);
    } 
    else 
    {
      if((panel.containsKey("mirr") && panel["mirr"] == true) || (panel.containsKey("rot") && panel["rot"] > 0)) fillPanel(p, result);
      else fillPanel(p, input);
    }
    
  }

}
