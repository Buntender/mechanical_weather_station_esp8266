#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>  
#include <ArduinoJson.h>

#ifndef _QWEATHER_H_
// 声明
#define _QWEATHER_H_

int israining(String location, String key) {
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);    
    client->setInsecure();    
    HTTPClient https;  
    String url="https://devapi.qweather.com/v7/weather/24h?lang=en&location="+location+"&key="+key+"&gzip=n";
    Serial.print("[HTTPS] begin...\n");  
    Serial.println(url);
    if (https.begin(*client, url)) {  // HTTPS    
      // start connection and send HTTP header  
      int httpCode = https.GET();    
      // httpCode will be negative on error  
      if (httpCode > 0) {  
        // HTTP header has been send and Server response header has been handled  
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);    
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {  
          DynamicJsonDocument doc(8192);
          DeserializationError error = deserializeJson(doc, https.getStream());
          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return -1;
          }
          int pred = 0;
          int lbl = 0;
          for (JsonObject hourly_item : doc["hourly"].as<JsonArray>()) {
            Serial.print((const char*)hourly_item["fxTime"]);
            Serial.print(" : ");
            int ratio = atoi((const char*)hourly_item["pop"]);
            Serial.print(ratio);
            Serial.println(" %");
            if(pred >= 6) break;
            pred++;
            if(ratio >= 30) lbl = 1;
          }
          return lbl;
        }  
      } else {  
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
         return -1;
      }  
      https.end();  
    } else {  
      Serial.printf("[HTTPS] Unable to connect\n");
      return -1;
    }
}

#endif
