#include <Arduino.h>
#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#include "FastLED.h"
#include <WiFiUdp.h>
#include "main.h"
#include "WebServer.h"
#include "Wire.h"
#include <Adafruit_AM2320.h>
#include <EEPROM.h>
#include <Adafruit_I2CDevice.h>
#include <IRrecv.h>
#include "LittleFS.h"


TwoWire i2c;

Adafruit_AM2320 AM2320(&i2c);

WiFiUDP UDP;
char packet[255];
int last_packet_len = 0;


CRGB currentColor = 0;
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

uint8_t filterArray[512];
uint16_t filterIndex = 0;
uint8_t lastMotionSensVal = 0;


float CurrentTemperature = 200;
float CurrentHumidity = 200;
int CurrentLedState = 0;
int CurrentDutyCycle = 0;
unsigned long CurrentTime = 0;
unsigned long debug_count = 0;
unsigned long SecondsCount = 0;
unsigned long lastMovementDetected = 0;
bool MovemenTrigger = false;

bool MovementSensorToggle = false;

int debug_pwm = 0;
unsigned long last_trigger_millis = 0;
enum LED_STATE_e{
  TURNING_ON = 100,
  TURNING_OFF,
  PALETTE_MODE,
  NORMALE_MODE
};

struct EEPROM_Save_s{
    int led_state;
    int duty;
    int palette;    
}SAVE_EEP;


int paletteMode = 0;
uint8_t startIndex = 0;

const uint16_t IRLEDPIN = D4;
IRrecv irReceiver(IRLEDPIN);
decode_results irResults;



void LEDS_Manager(void);
void SensorsManager();
void SetCurrentColor(CRGB color);
void NetworkManager();

void setup() {
  

Serial.begin(9600);
EEPROM.begin(256);
ESP.wdtEnable(3000);
i2c.begin(D2,D1);
i2c.beginTransmission(0x5C);

if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
// FSInfo info_fs;
// LittleFS.info(info_fs);

// Serial.println("File sistem info.");

// Serial.print("Total space:      ");
// Serial.print(info_fs.totalBytes);
// Serial.println("byte");

// Serial.print("Total space used: ");
// Serial.print(info_fs.usedBytes);
// Serial.println("byte");

// Serial.print("Block size:       ");
// Serial.print(info_fs.blockSize);
// Serial.println("byte");

// Serial.print("Page size:        ");
// Serial.print(info_fs.totalBytes);
// Serial.println("byte");

// Serial.print("Max open files:   ");
// Serial.println(info_fs.maxOpenFiles);

// Serial.print("Max path length:  ");
// Serial.println(info_fs.maxPathLength);

// Serial.println();

  if (!AM2320.begin())
  {
    Serial.println("Sensor not found");
  }

FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
FastLED.setBrightness(  BRIGHTNESS );
IPAddress local_IP(192, 168, 1, 16);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);



if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
  Serial.println("STA Failed to configure");
}


  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
  }

  if(UDP.begin(UDP_PORT)){
    Serial.println(UDP.localPort());
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  InitHTTPServer();

  FastLED.clear();
  SetCurrentColor(CRGB::Black);
  //PWM LEDS
  pinMode(D6, INPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  //digitalWrite(D6,HIGH);
   

   //irReceiver.enableIRIn(false);
  
  //HC-S501
  //pinMode(D1,INPUT);
  CurrentDutyCycle = 0;
  debug_count = millis();
}

void loop() {
  
  ESP.wdtFeed();
  LEDS_Manager();
  SensorsManager(); 
  NetworkManager();
}


void ExecCmd(const char* cmd)
{
  if(strstr(LEDS_PWM_ON,cmd) != NULL){
    CurrentLedState = TURNING_ON;
  }
  if(strstr(LEDS_PWM_OFF,cmd) != NULL){
    CurrentLedState = TURNING_OFF;
  }
  if(strstr(LEDS_PWM_COLOR,cmd) != NULL){
    //PARSE COLOR
  }
  if(strstr(MOVEMENTSENSOROFF,cmd) != NULL){
    MovementSensorToggle = false;
  }
  if(strstr(MOVEMENTSENSORON,cmd) != NULL){
    MovementSensorToggle = true;
  }
  if(strstr(GET_TEMP,cmd) != NULL){
    //PARSE COLOR
    UDP.beginPacket(UDP.remoteIP(),UDP.remotePort());
        char temp_buf[20];
        sprintf(temp_buf,"Temp: %f",AM2320.readTemperature());
        
        UDP.write(temp_buf);
        UDP.endPacket();
    
  }
  if(strstr(GET_HUMIDITY,cmd) != NULL){
    UDP.beginPacket(UDP.remoteIP(),UDP.remotePort());
        char temp_buf[20];
        sprintf(temp_buf,"Humidity: %f",AM2320.readHumidity());
        
        UDP.write(temp_buf);
        UDP.endPacket();
    
  }
  memset(packet,0,sizeof(packet));
}

void LEDS_Manager(void){
  if(CurrentLedState == TURNING_ON)
  {      
    for(int i = 0; i<NUM_LEDS; i++){
        leds[i] = currentColor;
        FastLED.setBrightness(255);
        FastLED.show();        
      }
      
      CurrentLedState = 0;
      /*if(CurrentDutyCycle == 0){
        CurrentTime = millis();
      }

      if((millis() - CurrentTime) < TIME_LED_TRANSACTION){
        FastLED.setBrightness(((millis() - CurrentTime) * 255) / TIME_LED_TRANSACTION);
        FastLED.show(); 
      }else{
        CurrentLedState = 0;
        CurrentTime = 0;
      }*/
  }

  if(CurrentLedState == TURNING_OFF)
  {
    for(int i = 0; i<NUM_LEDS; i++){
        leds[i] = CRGB::Black;
        FastLED.setBrightness(255);
        FastLED.show();        
      }
      CurrentLedState = 0;
  }

  if(CurrentLedState == PALETTE_MODE){   
    if((millis() - CurrentTime) > UPDATES_PER_SECOND){
      startIndex +=  1;
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = ColorFromPalette(currentPalette, startIndex);        
      }
      CurrentTime = millis();
      FastLED.show(); 
    }
     
     
  }
  
}

void FadeLed(uint8_t val){
  for(int i = 0; i < NUM_LEDS; i++ )
   {   
    leds[i] = currentColor;
    leds[i].maximizeBrightness(val);
  }
  FastLED.show();
}

void SetCurrentColor(CRGB color){
  currentColor = color;
  ExecCmd(LEDS_PWM_ON);
}

void PaletteModes(int val)
{
  Serial.println(val);
  if(val){
    switch(val){
      case 1: currentPalette = RainbowColors_p; break;
      case 2: currentPalette = LavaColors_p; break;
      case 3: currentPalette = CloudColors_p; break;
      case 4: currentPalette = OceanColors_p; break;
      case 5: currentPalette = ForestColors_p; break;
      case 6: currentPalette = PartyColors_p; break;
      case 7: currentPalette = HeatColors_p; break; 
    }
   
    currentBlending = LINEARBLEND;
    CurrentLedState = PALETTE_MODE;
    Serial.println(val);
  }else{
    
    currentBlending = LINEARBLEND;
    CurrentLedState = TURNING_ON;
  }
    
}



float GetTemperature()
{
  return CurrentTemperature;
}

float GetHumidity()
{
  return CurrentHumidity;
}

void SensorsManager()
{
  if((millis() - debug_count) > 30000)
  {
    

    if((AM2320.readTemperature() > -20) && (AM2320.readTemperature() < 50)){
    CurrentTemperature = AM2320.readTemperature();
    }  

    if((AM2320.readHumidity() > 0.0) && (AM2320.readHumidity() < 100.0)){
    CurrentHumidity = AM2320.readHumidity();
    }  
    debug_count = millis();
  }

  if (MovementSensorToggle)
  {
    Serial.println(digitalRead(D6));
    delay(100);
    if (digitalRead(D6))
    {
    MovemenTrigger = 1;
    if (lastMovementDetected == 0)
    {
        lastMovementDetected = millis();        
        FastLED.show();
        Serial.println("Detected");
        
    }
    }

    if (MovemenTrigger && lastMovementDetected != 0)
    {
    if ((millis() - lastMovementDetected) > 15000)
    {
        MovemenTrigger = 0;
        lastMovementDetected = 0;
        SetCurrentColor(CRGB::Black);
        FastLED.show();
        Serial.println("Not Detected");
        
    }
    }
  }


  /* Control air conditioner via ir ?? :) */
  /*
  if(irReceiver.decode(&irResults)){
     
    Serial.print("IR value: ");    
    Serial.printf("%X \r\n",irResults.value);    
    irReceiver.resume();
  }
  */
  
}

/*UDP LISTENER FOR RECEIVE CMD VIA UDP OPTIONALLY */

void NetworkManager()
{

if(WiFi.isConnected()){
  digitalWrite(LED_BUILTIN, HIGH);
}else{
  digitalWrite(LED_BUILTIN, LOW);
}
int packetSize = UDP.parsePacket();
  if (packetSize) {    
    int len = UDP.read(packet, 255);    
    if (len > 0)
    {
      packet[len] = '\0';
      last_packet_len = len;      
      if(CurrentLedState == 0){
        ExecCmd(packet);
        UDP.beginPacket(UDP.remoteIP(),UDP.remotePort());
        UDP.write("Received packet:\r\n");
        UDP.write(packet);
        UDP.endPacket();
      }
    }
  }
}

bool GetMovementSensorState(){
  return MovementSensorToggle;
}

