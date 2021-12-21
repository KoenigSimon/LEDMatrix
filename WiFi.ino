void callback(char* topic, byte* payload, unsigned int length) {

  char data[length + 1];
  for (int i = 0; i < length; i++) {
    data[i] = (char)payload[i];
  }
  data[length] = '\0';

  Serial.println("Message Received");
  Serial.print(data);
  Serial.println();


  error = deserializeJson(doc, (const byte*)payload);
  //error = deserializeJson(doc, data);

  if (!error)
  {
    newDataReady = true;
  }
  else
  {
    StatusLed[0] =  CRGB::Purple;
    FastLED.show();
  }

  Serial.print("Deserialisation State: ");
  Serial.println(error.c_str());
}

void wifi_setup()
{
  //network 1
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.begin(SSID1, WIFI_PASS);
  //WiFi.setNoDelay(false);

  bool ledOn = false;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (ledOn) {
      StatusLed[0] =  CRGB::Black;
      FastLED.show();
      ledOn = false;
    }
    else {
      StatusLed[0] =  CRGB::Red;
      FastLED.show();
      ledOn = true;
    }
    delay(100);
  }

  //setSingleLED(false, 0, CRGB::Yellow);
  //setSingleLED(true, 0, CRGB::Yellow);
  //FastLED.show();

  //try to update
  //t_httpUpdate_return ret = ESPhttpUpdate.update(UPDATE_SERVER, UPDATE_PORT, "/update.bin");

  //setSingleLED(false, 0, CRGB::Black);
  //setSingleLED(true, 0, CRGB::Black);
  //FastLED.show();
}

void manualUpdate()
{
  
  StatusLed[0] =  CRGB::Yellow;
  FastLED.show();

  //try to update
  //t_httpUpdate_return ret = ESPhttpUpdate.update(UPDATE_SERVER, UPDATE_PORT, "/update.bin");

  StatusLed[0] =  CRGB::Black;
  FastLED.show();

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect(MQTTNAME)) {
      // ... and resubscribe
      client.subscribe(MYTOPIC);
    } else {

      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2.5 seconds");

      // Wait 5 seconds before retrying
      StatusLed[0] =  CRGB::Green;

      FastLED.show();
      delay(2500);
    }
    StatusLed[0] =  CRGB::Black;
    FastLED.show();
  }
}
