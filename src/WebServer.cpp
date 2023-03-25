
#include "main.h"
#include "ESPAsyncWebServer.h"
#include "Arduino.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "WebServer.h"
#include "LittleFS.h"

volatile uint8_t requestProcessingBusy = 0;

AsyncWebServer server(80);

void WebPageShow(AsyncWebServerRequest *request);

File webpage;

String processor(const String& var);

void InitHTTPServer(void)
{
    


    server.on("/", WebPageShow);
    server.on("/lightbulb.png",HTTP_ANY,[](AsyncWebServerRequest *request) {
        request->send(LittleFS,"/lightbulb.png","image/png");
    });

    server.on("/thermostatt.png",HTTP_ANY,[](AsyncWebServerRequest *request) {
        request->send(LittleFS,"/thermostatt.png","image/png");
    });

    server.on("/humidity.png",HTTP_ANY,[](AsyncWebServerRequest *request) {
        request->send(LittleFS,"/humidity.png","image/png");
    });

    server.on("/testcolor",HTTP_GET,[](AsyncWebServerRequest *request) {
        Serial.println("testcolor POST:");
        for(int i = 0; i < request->params();i++){
            Serial.println(request->getParam(i)->value().c_str());
        }
    });
    
    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->send(404); });
    
    

    server.on("/buttons", HTTP_GET,[](AsyncWebServerRequest *request) {
        if(requestProcessingBusy == 0){
            requestProcessingBusy = 1;
        if(request->hasParam("range")){
            int rangeval = atoi(request->getParam("range")->value().c_str());
            if(rangeval > 0 && rangeval < 256){
                Serial.println(rangeval);
                FadeLed(rangeval);
            }            
            
        }

        if(request->hasParam("picker")){ 
            
            char hexcol[16];
            strcpy(hexcol,request->getParam("picker")->value().c_str());
            char red[3];
            red[0] = hexcol[0];
            red[1] = hexcol[1];
            red[2] = 'z';
            char green[3];
            green[0] = hexcol[2];
            green[1] = hexcol[3];
            green[2] = 'z';
            char blue[3];
            blue[0] = hexcol[4];
            blue[1] = hexcol[5];
            blue[2] = 'z';
            int rednum = (int)strtol(red, NULL, 16); 
            int greennum = (int)strtol(green, NULL, 16); 
            int bluenum = (int)strtol(blue, NULL, 16);              
            CRGB parsedcolor = CRGB(rednum,greennum,bluenum);            
            SetCurrentColor(parsedcolor);
        }

        if(request->hasParam("palette")){
            
            int val = atoi(request->getParam("palette")->value().c_str());
            
            PaletteModes(val);            
        }

        if(request->hasParam("LED")){            
            int val = atoi(request->getParam("LED")->value().c_str());
            if (val)
            {
                Serial.println("LED ON");
                ExecCmd(LEDS_PWM_ON);
            }
            else
            {
                Serial.println("LED OFF");
                ExecCmd(LEDS_PWM_OFF);
            }
        }

        if(request->hasParam("MoveSens")){
            int val = atoi(request->getParam("MoveSens")->value().c_str());
            if(val)
            {
                Serial.println("Sensor Movement ON");
                ExecCmd(MOVEMENTSENSORON);
            }else{
                Serial.println("Sensor Movement OFF");
                ExecCmd(MOVEMENTSENSOROFF);
            }
        }

        WebPageShow(request);
        requestProcessingBusy = 0;
        }
    });
 
    //server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started");
}



void WebPageShow(AsyncWebServerRequest *request){  
    
    
    webpage = LittleFS.open("/index.html", "r");

    
    if (webpage.available())
    {
        
        //sprintf(BufferMemory, webpage.readString().c_str(), tem, hum, mov, hr, min % 60, sec % 60);
        //printf(webpage.readString().c_str(),tem, hum, mov, hr, min % 60, sec % 60);
        request->send(LittleFS, "/index.html", String(),false, processor);
        webpage.close();
    }
    

    //request->send(200, "text/html","OK");
  
}

String processor(const String& var){  
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  int tem = (int)GetTemperature();
  int hum = (int)GetHumidity();
  int mov = (int)GetMovementSensorState();
  if(var == "TEMPERATURE"){
    
    return String(tem);
  }
  if(var == "HUMI"){
    return String(hum);
  }
  if(var == "MOV"){
    return String(mov);
  }
  if(var == "HOURS"){
    return String(hr);
  }
  if(var == "MIN"){
    return String(min);
  }
  if(var == "SEC"){
    return String(sec);
  }
  return String();
}


