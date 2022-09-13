/**The MIT License (MIT)

Copyright (c) 2018 by Daniel Eichhorn - ThingPulse

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

See more at https://thingpulse.com
*/

//Monsteryuan forked from Daniel Eichhorn/ThingPulse ESP8266 Weather Station

#include <Arduino.h>

// time
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()

#include "wificonnect.h"
#include "qweather.h"
#include "servo.h"

#include "Ticker.h"

/***************************
 * Begin Settings
 **************************/

#define TZ              8       // (utc+) TZ in hours
#define DST_MN          0      // use 60mn for summer time in some countries

const int SERVO_PIN = 0;

/***************************
 * End Settings
 **************************/

#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

#define VALIDTIME(TIME) (TIME > 1576800000)
#define GETINVALIDTIME(TIME) (TIME - 1576800000)

String QKEY="Get QKeys From qweather.com";
String QLOCATION="Your coordinates max accuracy 0.01"; \\ eg "113.30,23.10"
servo* myservo;
int NEXTSTATUSREADY = 0;

void setup() {
  Serial.begin(115200);
  myservo = new servo(SERVO_PIN);
  myservo->trans(MID);
  
  auto stepforwardtrigger = []() -> void { NEXTSTATUSREADY = 1;};
  auto stepforwardwrapper = []() -> void {
    if(NEXTSTATUSREADY){
      myservo->stepforward();
      NEXTSTATUSREADY = 0;
    }
  };
  
  Ticker* scanpin = new Ticker;
  scanpin->attach(3, stepforwardtrigger);

  while(!VALIDTIME(time(nullptr))){
    WiFi.mode(WIFI_OFF);
    if(lastConfig() == false){
      htmlConfig(stepforwardwrapper);//HTML配网
    }
    Serial.println("");
    configTime(TZ_SEC, DST_SEC, "pool.ntp.org","0.cn.pool.ntp.org","1.cn.pool.ntp.org");
    for(int timeout = 600; timeout > 0; timeout--){
        if(VALIDTIME(time(nullptr))) break;
        delay(500);
        Serial.print(".");
        stepforwardwrapper();
    }
  }

  delete scanpin;
  
  int res = israining(QLOCATION, QKEY);
  Serial.println(res);
  if(res == 1) myservo->trans(POS);
  else if(res == 0) myservo->trans(NEG);
  else myservo->trans(MID);
}

bool isrighttime(long currenttime){
//  Serial.println(currenttime);
  
  if(currenttime - (6 * 3600 + 30 * 60) < 6 * 60 && currenttime - (6 * 3600 + 30 * 60) > -6 * 60) return true;
  if(currenttime - (11 * 3600 + 30 * 60) < 6 * 60 && currenttime - (11 * 3600 + 30 * 60) > -6 * 60) return true;
  if(currenttime - (13 * 3600 + 30 * 60) < 6 * 60 && currenttime - (13 * 3600 + 30 * 60) > -6 * 60) return true;
  if(currenttime - (17 * 3600) < 6 * 60 && currenttime - (17 * 3600) > -6 * 60) return true;
  if(currenttime - (21 * 3600 + 30 * 60) < 6 * 60 && currenttime - (21 * 3600 + 30 * 60) > -6 * 60) return true;
  return false;
}

long getLocalTime()
{
  time_t now;
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now);  
  Serial.print(timeinfo->tm_hour);
  Serial.print(":");
  Serial.print(timeinfo->tm_min);
  Serial.print(":");
  Serial.println(timeinfo->tm_sec);
  return timeinfo->tm_hour * 3600 + timeinfo->tm_min * 60 + timeinfo->tm_sec;
}

void getprepared(){
  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting to WIFI network");
      WiFi.disconnect();
      lastConfig();
    }
    if(VALIDTIME(time(nullptr))){
      time_t rtc = GETINVALIDTIME(time(nullptr));
      timeval tv = { rtc, 0 };
      settimeofday(&tv, nullptr);
    }
    configTime(TZ_SEC, DST_SEC, "pool.ntp.org","0.cn.pool.ntp.org","1.cn.pool.ntp.org");
    for(int timeout = 600; timeout > 0; timeout--){
        if(VALIDTIME(time(nullptr))) break;
        delay(500);
        Serial.print(".");
    }
}

void loop() {
  if (isrighttime(getLocalTime())) {
    WiFi.forceSleepWake(); // Wifi on
    yield();
    int retry = 3, res = -1;
    do{
      // checking for WIFI connection
      getprepared();
      for(int retryconnect = 3; retryconnect > 0; retryconnect--){
          res = israining(QLOCATION, QKEY);
          if(res != -1){
            retry = 0;
            break;
          }
          else delay(10 * 1000);
      }
      retry--;
      WiFi.disconnect();
      delay(3 * 60 * 1000);
    }while(retry > 0);
    if(res == 1) myservo->trans(POS);
    else if(res == 0) myservo->trans(NEG);
    else myservo->trans(MID);
  }
  
  WiFi.forceSleepBegin(); // Wifi off
  delay(10 * 60 * 1000);
}
