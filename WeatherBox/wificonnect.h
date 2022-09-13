#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "savenet.h"

#ifndef _WIFICONNECT_H_
// 声明
#define _WIFICONNECT_H_

bool lastConfig()
{
  WiFi.mode(WIFI_STA);
  WifiPwd* wifipwd = loadConfigs();
  if(strlen(wifipwd->pwd) > 0){
    WiFi.begin(wifipwd->ssid, wifipwd->pwd);
    Serial.print("AutoConfig Waiting......");
    for (int i = 0; i < 20; i++)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println("AutoConfig Success");
        Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
        Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
        Serial.print("IP address:");
        Serial.println(WiFi.localIP());
        WiFi.printDiag(Serial);
        delete wifipwd;
        return true;
      }
      else
      {
         delay(500);
         Serial.print(".");
      }
    }
  }
  Serial.println("AutoConfig Faild!" );
  delete wifipwd;
  return false;
}

ESP8266WebServer server(80);
String HTML_TITLE = "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"><title>ESP8266网页配网</title>";
String HTML_SCRIPT_ONE = "<script type=\"text/javascript\">function wifi(){var ssid = s.value;var password = p.value;var xmlhttp=new XMLHttpRequest();xmlhttp.open(\"GET\",\"/HandleWifi?ssid=\"+ssid+\"&password=\"+password,true);xmlhttp.send();xmlhttp.onload = function(e){alert(this.responseText);}}</script>";
String HTML_SCRIPT_TWO = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
String HTML_HEAD_BODY_BEGIN = "</head><body>请输入wifi信息进行配网:";
String HTML_FORM_ONE = "<form>WiFi名称：<input id='s' name='s' type=\"text\" placeholder=\"请输入您WiFi的名称\"><br>WiFi密码：<input id='p' name='p' type=\"text\" placeholder=\"请输入您WiFi的密码\"><br><input type=\"button\" value=\"扫描\" onclick=\"window.location.href = '/HandleScanWifi'\"><input type=\"button\" value=\"连接\" onclick=\"wifi()\"></form>";
String HTML_BODY_HTML_END = "</body></html>";

void handleRoot() {
    Serial.println("root page");
    String str = HTML_TITLE + HTML_SCRIPT_ONE + HTML_SCRIPT_TWO + HTML_HEAD_BODY_BEGIN + HTML_FORM_ONE + HTML_BODY_HTML_END;
    server.send(200, "text/html", str);
}

void HandleScanWifi() {
    Serial.println("scan start");

    String HTML_FORM_TABLE_BEGIN = "<table><head><tr><th>序号</th><th>名称</th><th>强度</th></tr></head><body>";
    String HTML_FORM_TABLE_END = "</body></table>";
    String HTML_FORM_TABLE_CON = "";
    String HTML_TABLE;
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
        HTML_TABLE = "NO WIFI !!!";
    }
    else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
            delay(10);
            HTML_FORM_TABLE_CON = HTML_FORM_TABLE_CON + "<tr><td align=\"center\">" + String(i+1) + "</td><td align=\"center\">" + "<a href='#p' onclick='c(this)'>" + WiFi.SSID(i) + "</a>" + "</td><td align=\"center\">" + WiFi.RSSI(i) + "</td></tr>";
        }

        HTML_TABLE = HTML_FORM_TABLE_BEGIN + HTML_FORM_TABLE_CON + HTML_FORM_TABLE_END;
    }
    Serial.println("");

    String scanstr = HTML_TITLE + HTML_SCRIPT_ONE + HTML_SCRIPT_TWO + HTML_HEAD_BODY_BEGIN + HTML_FORM_ONE + HTML_TABLE + HTML_BODY_HTML_END;

    server.send(200, "text/html", scanstr);
}

void HandleWifi()
{
    String wifis = server.arg("ssid"); //从JavaScript发送的数据中找ssid的值
    String wifip = server.arg("password"); //从JavaScript发送的数据中找password的值
    Serial.println("received:"+wifis);
    server.send(200, "text/html", "连接中..");
    WiFi.begin(wifis,wifip);
}

void handleNotFound() { 
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

auto NULLFUNC = []() -> void {};

void htmlConfig(void(loading)(void) = NULLFUNC)
{
    WiFi.mode(WIFI_AP_STA);//设置模式为AP+STA
    WiFi.softAP("WeatherCube");

    IPAddress myIP = WiFi.softAPIP();
  
    if (MDNS.begin("clock")) {
      Serial.println("MDNS responder started");
    }
    
    server.on("/", handleRoot);
    server.on("/HandleWifi", HTTP_GET, HandleWifi);
    server.on("/HandleScanWifi", HandleScanWifi);
    server.onNotFound(handleNotFound);//请求失败回调函数
    MDNS.addService("http", "tcp", 80);
    server.begin();//开启服务器
    Serial.println("HTTP server started");
    while(1)
    {
        server.handleClient();
        MDNS.update();  
        delay(500);
        loading();
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("HtmlConfig Success");
            Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
            Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
            Serial.println("HTML连接成功");
            WifiPwd *wifipwd  = new WifiPwd;
            strcpy(wifipwd->ssid, WiFi.SSID().c_str());
            strcpy(wifipwd->pwd, WiFi.psk().c_str());
            saveConfig(wifipwd);
            break;
        }
    }
    server.close();  
    WiFi.mode(WIFI_STA);   
}


#endif
