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

unsigned long errLEDTimer = 0;

float brightnessFactor = 1;

unsigned long lastDecodeTime;
unsigned long lastFetchTime;
CRGB StatusLed[1];

void setup() {
  //turn off leds
  Serial.begin(1500000);
  Serial.setRxBufferSize(8192);
  
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

void loop() {
  if(!digitalRead(BUTTON)){
    while(!digitalRead(BUTTON));
    currentprof++;
  }  

  //read data from wifi
  if (millis() > connectionTimer + 500)
  {
    connectionTimer = millis();
    if (!client.connected())
      reconnect();

    client.loop();
  }

  //read data from serial  
  if(Serial.available())
  {
    lastFetchTime = micros();
    DeserializationError err = deserializeJson(doc, Serial);

    if (err == DeserializationError::Ok) 
    {
      lastFetchTime = micros() - lastFetchTime;
      ReadData();

      writeTelemetry();
    } 
    else 
    {     
      errLEDTimer = millis() + 50;
      Serial.flush();
    }
    
  }

  //on error show led
  if(errLEDTimer > millis())
  {
    StatusLed[0] =  CRGB::Purple;
      FastLED.show();
  } else {
    StatusLed[0] =  CRGB::Black;
      FastLED.show();
  }
  

  if (newDataReady)
  {
    newDataReady = false;
    ReadData();

    writeTelemetry();
  }
  
  FastLED.show();
}

void writeTelemetry()
{
  client.publish("dev/protoHead/decodeTime", String(lastDecodeTime).c_str());
  client.publish("dev/protoHead/FetchTime", String(lastFetchTime).c_str());
}

void ReadData()
{
  //start measuering time data
  lastDecodeTime = micros();
  
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
      String temp = panel["dots"][i];
      if(temp.startsWith("0"))
        input[i] = strtol(panel["dots"][i], 0, 16);
      else 
        input[i] = strtol("0x000000", 0, 16);
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

  lastDecodeTime = micros() - lastDecodeTime;

}
