#include "clock_background.h"
#include "hour_niddle.h"
#include "minute_niddle.h"
#include "second_niddle.h"

#include <WiFiManager.h>
WiFiManager wm;

#include <ESP32Time.h>
ESP32Time rtc;

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

TFT_eSprite second_needle_sprite = TFT_eSprite(&tft); // Sprite object for needle
TFT_eSprite minute_needle_sprite = TFT_eSprite(&tft); // Sprite object for needle
TFT_eSprite hour_needle_sprite = TFT_eSprite(&tft); // Sprite object for needle
TFT_eSprite clock_back_sprite = TFT_eSprite(&tft); // Sprite object for needle

#define TFT_GREY 0x7BEF
#define Switch 21
#define BackLight 20

bool is_sleep_active = 0;

const char *bufferStr = R"(
  
  <!-- INPUT SELECT -->
  <br/>
  <label for='hour_select'>Choose Hour</label>
  <select name="hour_select" id="hour_select" class="button">
  <option value="0">--choose an option--</option>
  <option value="1">  1 </option>
  <option value="2">  2 </option>
  <option value="3">  3 </option>
  <option value="4">  4 </option>
  <option value="5">  5 </option>
  <option value="6">  6 </option>
  <option value="7">  7 </option>
  <option value="8">  8 </option>
  <option value="9">  9 </option>
  <option value="10">10 </option>
  <option value="11">11 </option>
  <option value="12">12 </option>
  </select>

  <br/>
  <label for='minute_select'>Choose minute</label>
  <select name="minute_select" id="minute_select" class="button">
  <option value="0">--choose an option--</option>
  <option value="1">   1 </option>
  <option value="2">   2 </option>
  <option value="3">   3 </option>
  <option value="4">   4 </option>
  <option value="5">   5 </option>
  <option value="6">   6 </option>
  <option value="7">   7 </option>
  <option value="8">   8 </option>
  <option value="9">   9 </option>
  <option value="10"> 10 </option>
  <option value="11"> 11 </option>
  <option value="12"> 12 </option>
  <option value="13"> 13 </option>
  <option value="14"> 14 </option>
  <option value="15"> 15 </option>
  <option value="16"> 16 </option>
  <option value="17"> 17 </option>
  <option value="18"> 18 </option>
  <option value="19"> 19 </option>
  <option value="20"> 20 </option>
  <option value="21"> 21 </option>
  <option value="22"> 22 </option>
  <option value="23"> 23 </option>
  <option value="24"> 24 </option>
  <option value="25"> 25 </option>
  <option value="26"> 26 </option>
  <option value="27"> 27 </option>
  <option value="28"> 28 </option>
  <option value="29"> 29 </option>
  <option value="30"> 30 </option>
  <option value="31"> 31 </option>
  <option value="32"> 32 </option>
  <option value="33"> 33 </option>
  <option value="34"> 34 </option>
  <option value="35"> 35 </option>
  <option value="36"> 36 </option>
  <option value="37"> 37 </option>
  <option value="38"> 38 </option>
  <option value="39"> 39 </option>
  <option value="40"> 40 </option>
  <option value="41"> 41 </option>
  <option value="42"> 42 </option>
  <option value="43"> 43 </option>
  <option value="44"> 44 </option>
  <option value="45"> 45 </option>
  <option value="46"> 46 </option>
  <option value="47"> 47 </option>
  <option value="48"> 48 </option>
  <option value="49"> 49 </option>
  <option value="50"> 50 </option>
  <option value="51"> 51 </option>
  <option value="52"> 52 </option>
  <option value="53"> 53 </option>
  <option value="54"> 54 </option>
  <option value="55"> 55 </option>
  <option value="56"> 56 </option>
  <option value="57"> 57 </option>
  <option value="58"> 58 </option>
  <option value="59"> 59 </option>
  <option value="60"> 60 </option>
  </select>
  
  )";


WiFiManagerParameter custom_html_inputs(bufferStr);

void setup() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  pinMode(Switch,INPUT_PULLUP);
  pinMode(BackLight,OUTPUT);
  digitalWrite(BackLight,HIGH);

  wm.addParameter(&custom_html_inputs);

  // callbacks
  wm.setSaveParamsCallback(saveParamCallback);
  
  //set title
  wm.setTitle("Set Time");

  // invert theme, dark
  wm.setDarkMode(true);

  std::vector<const char *> menu = {"param", "sep", "exit"};
  wm.setMenu(menu); // custom menu, pass vector
  
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
//  tft.setPivot(20 ,93);
//  tft.setSwapBytes(true);
//  tft.pushImage(0, 0, 240, 240, background);
//  tft.pushImage(100, 25, 13, 110, minute_niddle);

  create_second_niddle();
  create_minute_niddle();
  create_hour_niddle();

  //enter in setting mode
  if(digitalRead(Switch)){
    setting_mode_display();
    wm.autoConnect("digiPclock");
  }
  
//  time_map(12, 60, 60);
}

//______________________________ LOOP ___________________________________________

void loop() {
    if(digitalRead(Switch)){
      if(is_sleep_active == 1){
        digitalWrite(BackLight,HIGH);
        is_sleep_active = 0;
      }
      time_map(rtc.getHour(), rtc.getMinute(), rtc.getSecond());
    }
    else{
      if(is_sleep_active == 0){
        is_sleep_active = 1;
        going_to_sleep();
        gpio_wakeup_enable(GPIO_NUM_21,GPIO_INTR_HIGH_LEVEL);
        esp_sleep_enable_gpio_wakeup();
        delay(2000);
        tft.fillScreen(TFT_BLACK);
        digitalWrite(BackLight,LOW);
        gpio_hold_en(GPIO_NUM_20);
        delay(100);
        esp_light_sleep_start();
        esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
        gpio_hold_dis(GPIO_NUM_20);
      }
    }
}

//______________________________ FUNCTIONS _________________________________________
void time_map(int hour, int minute, int second){

  ///////Handaling Wrong time//////
  if(hour > 12 || hour < 1){
    hour = 12;
  }
  if(minute > 60 || minute < 1){
    minute = 60;
  }
  if(second > 60 || second < 1){
    second = 60;
  }

  ///////mapping time//////
  int16_t hour_angle = map(hour, 0,12 , 0,360 );
  int16_t minute_angle = map(minute, 0,60 , 0,360 );
  int16_t second_angle = map(second, 0,60 , 0,360 );

  /////moving hour hand towaords next hour////
  if(minute >= 15 && minute < 30){
    hour_angle = hour_angle + 7;
  }
  else if(minute >= 30 && minute < 45){
    hour_angle = hour_angle + 14;
  }
  else if(minute >= 45 && minute < 60){
    hour_angle = hour_angle + 21;
  }
  
  createBackground();

  clock_back_sprite.setPivot(119, 181);
  second_needle_sprite.pushRotated(&clock_back_sprite, second_angle,TFT_TRANSPARENT);
  
  clock_back_sprite.setPivot(119, 118);
  hour_needle_sprite.pushRotated(&clock_back_sprite, hour_angle,TFT_TRANSPARENT);
  minute_needle_sprite.pushRotated(&clock_back_sprite, minute_angle,TFT_TRANSPARENT);
  
  clock_back_sprite.pushSprite(0,0,TFT_TRANSPARENT);
}

void createBackground(){
//  clock_back_sprite.setColorDepth(5);
  clock_back_sprite.createSprite(240,240);
//  clock_back_sprite.setPivot(119, 118); //for minute,hour
//  clock_back_sprite.setPivot(119, 181); //for second
  clock_back_sprite.fillSprite(TFT_TRANSPARENT);
  clock_back_sprite.setSwapBytes(true);
  clock_back_sprite.pushImage(0,0,240,240,background);
}

void create_minute_niddle(){
  minute_needle_sprite.createSprite(9,102);
  minute_needle_sprite.setSwapBytes(true);
  minute_needle_sprite.pushImage(0,0,9,110,minute_niddle);
  minute_needle_sprite.setPivot(4 ,110);
}

void create_hour_niddle(){
  hour_needle_sprite.createSprite(13,60);
  hour_needle_sprite.setSwapBytes(true);
  hour_needle_sprite.pushImage(0,0,13,60,hour_needle);
  hour_needle_sprite.setPivot(6 ,69);
}

void create_second_niddle(){
  second_needle_sprite.createSprite(9,25);
  second_needle_sprite.setSwapBytes(true);
  second_needle_sprite.pushImage(0,0,9,30,second_needle);
  second_needle_sprite.setPivot(4 ,29);
}

void setting_mode_display(){
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, 319, 65,TFT_GREY);
  
  tft.setFreeFont(&FreeMonoBold12pt7b);    
  tft.setTextColor(TFT_BLACK, TFT_GREY);
  tft.drawString("SET TIME MODE", 28, 40, 1);

  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Connect To", 67, 80, 1);
  
  tft.setFreeFont(&FreeMonoBold12pt7b);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString("digiPclock", 50, 102, 1);

  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("WIFI", 102, 128, 1);
  tft.drawString("To Set The Time", 39, 148, 1);

//  tft.fillRect(0, 200, 319, 62,TFT_GREY);
//  tft.setTextColor(TFT_BLACK, TFT_GREY);
//  tft.drawString("by", 110, 185, 1);
//  tft.drawString("vishalsoniindia", 50, 210, 1);
}

void going_to_sleep(){
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(&FreeMonoBold12pt7b);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Going to Sleep", 20, 102, 1);
}

void saveParamCallback() {
//  hour    = getParam("hour_select").toInt();
//  minute  = getParam("minute_select").toInt();
//  rtc.setTime(30, 24, 15, 17, 1, 2042);  // 17th Jan 2021 15:24:30
  rtc.setTime(00, getParam("minute_select").toInt(), getParam("hour_select").toInt(), 12, 2, 2023);  // 17th Jan 2021 15:24:30
  wm.stopConfigPortal();
}

String getParam(String name) {
  //read parameter from server, for customhmtl input
  String value;
  if (wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}
