#include <Arduino.h>
#include <SPI.h>
#include "IoTMakers.h"
#include "Shield_Wrapper.h"
#include "Timer.h"

/*
Arduino Shield
*/
Shield_Wrapper  g_shield;

#define SDCARD_CS 4


void sdcard_deselect()
{
  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, HIGH); //Deselect the SD card
}
void init_shield()
{
  sdcard_deselect();
  
  const char* WIFI_SSID = "write your wifi_ssid";
  const char* WIFI_PASS = "write your wifi_pass";
  g_shield.begin(WIFI_SSID, WIFI_PASS);

  g_shield.print();
}


/*
IoTMakers
*/
IoTMakers g_im;

const char deviceID[]   = "kt iotmakers's deviceID";
const char authnRqtNo[] = "xxxxxxxx";
const char extrSysID[]  = "OPEN_TCP_001PTL00xxxxxxxx";

void init_iotmakers()
{
  Client* client = g_shield.getClient();
  if ( client == NULL ) {
    Serial.println(F("No client from shield."));
    while(1);
  }

  g_im.init(deviceID, authnRqtNo, extrSysID, *client);
  g_im.set_numdata_handler(mycb_numdata_handler);
  g_im.set_strdata_handler(mycb_strdata_handler);
  g_im.set_dataresp_handler(mycb_resp_handler);

  // IoTMakers 서버 연결
  Serial.println(F("connect()..."));
  while ( g_im.connect() < 0 ){
    Serial.println(F("retrying."));
    delay(3000);
  }

  // Auth

  Serial.println(F("auth."));
  while ( g_im.auth_device() < 0 ) {
    Serial.println(F("fail"));
    while(1);
  }

  //Serial.print(F("FreeRAM="));Serial.println(g_im.getFreeRAM());
}
int WaterLevel1 = analogRead(0);
int WaterLevel2 = analogRead(1);
float WaterLevelAverange = (WaterLevel1 + WaterLevel2)/2;

int LEDGreen = 5;
int LEDYellow = 4;
int LEDRed = 3;
#define buzzerPin  8
int state = 0;
int k = 0;
Timer t;
const unsigned long PERIOD = 100;

void setup(void) 
{
  Serial.begin(115200);
    while ( !Serial )  {
    ;
  }
  pinMode(buzzerPin, OUTPUT);
  pinMode(LEDGreen, OUTPUT);
  digitalWrite(LEDGreen, HIGH);
  pinMode(LEDYellow, OUTPUT);
  digitalWrite(LEDYellow, HIGH);
  pinMode(LEDRed, OUTPUT);
  digitalWrite(LEDRed, HIGH);
  t.oscillate(LEDRed,PERIOD,HIGH);
  init_shield();
  
  init_iotmakers();

  digitalWrite(LEDGreen, LOW);
  digitalWrite(LEDYellow, LOW);
  digitalWrite(LEDRed, LOW);


}

void loop(void)
{
  int WaterLevel1 = analogRead(0);
  int WaterLevel2 = analogRead(1);
  float WaterLevelAverange = (WaterLevel1 + WaterLevel2)/2;
  if(WaterLevelAverange > 140){k = 1;}else {k = 0;}
  if( k == 1){
  t.update();
  buzzer();
  Serial.println("WARNNING!!WARNNING!!WARNNING!!WARNNING!!");
  k = 0;
  }
  static unsigned long tick = millis();
  // 5초 주기로 센서 정보 송신
  if ( ( millis() - tick) > 5000 )
  {
    send_waterlevel();

    tick = millis();
  }
  
  // IoTMakers 서버 수신처리 및 keepalive 송신
  g_im.loop();
   
}

int send_waterlevel()
{ 
  int WaterLevel1 = analogRead(0);
  int WaterLevel2 = analogRead(1);
  float WaterLevelAverange = (WaterLevel1 + WaterLevel2)/2;

  Serial.print(F("WaterLevel1 : ")); Serial.print(WaterLevel1);
  Serial.print(F("     WaterLevel2 : ")); Serial.print(WaterLevel2);
  Serial.print(F("     WaterLevelaverange : "));Serial.print(WaterLevelAverange); 
   
  if ( g_im.send_numdata("WaterLevel", (double)WaterLevelAverange) < 0) {
      Serial.println(F("fail"));  
    return -1;
  }
  return 0;   
}


void mycb_numdata_handler(char *tagid, double numval)
{
  // !!! USER CODE HERE
  //Serial.print(tagid);Serial.print(F("="));Serial.println(numval);
}

void mycb_strdata_handler(char *tagid, char *strval)
{
  //Green
  if ( strcmp(tagid, "LEDGreen")==0 && strcmp(strval, "ON")==0 ){   
    digitalWrite(LEDGreen, HIGH);
    delay(100);
    Serial.println("LEDGreen");
  }else if ( strcmp(tagid, "LEDGreen")==0 && strcmp(strval, "OFF")==0 ){   
    digitalWrite(LEDGreen, LOW);
  }
    //Yellow
  if ( strcmp(tagid, "LEDYellow")==0 && strcmp(strval, "ON")==0 ){   
    digitalWrite(LEDYellow, HIGH);
    delay(100);
    Serial.println("LEDYellow");
  }else if ( strcmp(tagid, "LEDYellow")==0 && strcmp(strval, "OFF")==0 ){   
    digitalWrite(LEDYellow, LOW);
  }
    //Red
  if ( strcmp(tagid, "LEDRed")==0 && strcmp(strval, "ON")==0 ){   
    digitalWrite(LEDRed, HIGH);
    delay(100);
    Serial.println("LEDRed");
  }else if ( strcmp(tagid, "LEDRed")==0 && strcmp(strval, "OFF")==0 ){   
    digitalWrite(LEDRed, LOW);
  }
}

void mycb_resp_handler(long long trxid, char *respCode)
{
  if ( strcmp(respCode, "100")==0 )
    Serial.println("resp:OK");
  else
    Serial.println("resp:Not OK");
}
void buzzer(){
for(int freq = 150; freq <=1800; freq = freq + 2) {
    tone(buzzerPin, freq, 10);
    delay(1);
  }
  for(int freq = 1800; freq <=150; freq = freq - 2) {
    tone(buzzerPin, freq, 10);
    delay(1);
  }
}
