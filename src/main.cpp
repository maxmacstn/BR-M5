#include "M5StickCPlus.h"
#include "CanonBLE.h"
#include "Display.h"
#include "TimeLapse_Management.h"
#include <Arduino.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include <esp32-hal-log.h>

String name_remote = "BR-M5";
Display M5_display(&M5.Lcd, name_remote);
CanonBLE canon_ble(name_remote);
TimeLapse timelapse(400);

enum RemoteMode {Settings, Shooting}current_mode;

void update_shooting()
{
    // Update timelapse
    if (timelapse.TimeLapse_Trigger())
    {
        canon_ble.trigger();
        Serial.println("Trigger tl");
    }

    // Remote control
    if (timelapse.get_interval() == 0) //Single shots
    {
        if (M5.BtnA.wasReleased() && !M5.BtnB.wasReleased())
        {
            canon_ble.trigger();
            Serial.println("Single shot");
        }
    }
    else // Timelapses
    {
        // Stop or start timelapse
        if (M5.BtnA.wasReleased())
        {
            if (timelapse.Recording_OnOFF())
            {
                Serial.println("Start timelapse");
                M5_display.set_main_menu_screen(timelapse.get_interval(), "Shooting timelapse");
            }
            else
            {
                Serial.println("Stop timelapse");
                M5_display.set_main_menu_screen(timelapse.get_interval(), "Ready for timelapse");
            }
        }
    }
}

void update_settings()
{
    if (M5.BtnA.wasReleased())
    {
        timelapse.TimeLapse_decDelay();
        M5_display.set_main_menu_screen(timelapse.get_interval(), "Setting interval");
    }
    if (M5.BtnB.wasReleased())
    {
        timelapse.TimeLapse_incDelay();
        M5_display.set_main_menu_screen(timelapse.get_interval(), "Setting interval");
    }
}

void connect_loop(){
    while (! canon_ble.is_ready_to_connect())
    {
        log_i("Scanning.");
        canon_ble.scan(5);
    }


    log_i("Canon device found: ");
    Serial.println(canon_ble.get_device_address().toString().c_str());
    log_i("Trying to connect.");
    if (canon_ble.connect_to_device())
    {
        log_i("Connected successfully");
        M5_display.set_address(canon_ble.get_device_address().toString().c_str());
        M5_display.set_main_menu_screen(timelapse.get_interval(), "Ready for single shot");
    }
    else { 
        log_e("failed to connect");
    }
}

void trigger_loop(){
    switch (current_mode)
    {
    case Settings:
        if (M5.BtnB.pressedFor(700))
        {
            M5.BtnB.reset();
            current_mode = Shooting;
            String status = (timelapse.get_interval()==0)?"Ready for single shot":"Ready for timelapse";
            M5_display.set_main_menu_screen(timelapse.get_interval(), status);
        }
        else
        {
            update_settings();
        }
        break;
    
    case Shooting:
        if (M5.BtnB.pressedFor(700))
        {
            M5.BtnB.reset();
            current_mode = Settings;
            M5_display.set_main_menu_screen(timelapse.get_interval(), "Setting interval");
        }
        else
        {
            update_shooting();
        }
        break;

    default:
        break;
    }
}

void try_connect() {
    bool connected = false;
    while (!connected) {
        Serial.println("Trying to connect.");
        if (canon_ble.connect_to_device())
        {
            Serial.println("Connected successfully");
            connected = true; 
        }
        else { 
            Serial.println("Failed to connect.");
        }
    }
}

void test_reconnect() {
    while (! canon_ble.is_ready_to_connect())
    {
        log_i("Scanning.");
        canon_ble.scan(5);
    }

    Serial.println(canon_ble.get_device_address().toString().c_str());

    for (int i = 0; i < 100; i++) {
        try_connect();
        delay(200);
        if (canon_ble.is_connected()) {
            log_i("connection successful");
        } else {
            log_e("connection failed");
        }

        delay(500);

        canon_ble.disconnect();
        delay(200);
        if (!canon_ble.is_connected()) {
            log_i("disconnection successful");
        } else {
            log_e("disconnection failed");
        }

        delay(500);
    }
}

void loop()
{
    // Update buttons state
    M5.update();

    // Check connection state
    // If disconnected, bump back to connecting
    if (canon_ble.is_connected()) {
        trigger_loop();
    }
    else {
        M5_display.set_init_screen();
        connect_loop();
    }

    
    delay(10);
}

void setup()
{
    esp_log_level_set("*", ESP_LOG_INFO); 

    M5.begin();
    current_mode = Shooting;
    M5.Axp.ScreenBreath(9);
    M5.Lcd.setRotation(1);
    M5_display.set_init_screen();

    connect_loop();
}