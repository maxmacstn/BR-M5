#include "CanonBLERemote.h"
#include <Arduino.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include <esp32-hal-log.h>

String name_remote = "ESP32 Remote";
CanonBLERemote canon_ble(name_remote);

void setup()
{
    Serial.begin(115200);
    esp_log_level_set("*", ESP_LOG_INFO); 

    pinMode(13, INPUT_PULLUP);
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);

    canon_ble.init();

    // pair() function should be called only when you want to pair with the new camera. 
    // After paired, the pair() function call should be ommited.


    if (digitalRead(13) == LOW){
        while(!canon_ble.pair(10)){
            Serial.println("Pairing...");
        }
    }
    // canon_ble.pair(10);

    delay(1000);
    Serial.println("Setup Done");
}

void loop()
{

    if (digitalRead(13) == LOW){

        Serial.println("Shutter pressed");
        digitalWrite(2, LOW);
    
        if( canon_ble.trigger()){
            Serial.println("Trigger Success");

        }else{
            Serial.println("Trigger Failed");
        }

        digitalWrite(2, HIGH);

    }
}

