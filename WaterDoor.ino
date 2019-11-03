#include <Arduino.h>
#include <SPI.h>
#include "IoTMakers.h"
#include "Shield_Wrapper.h"
#include "Timer.h"
#include <Servo.h>
/*sdsdsdsda
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
  
  const char* WIFI_SSID = "iptime";
  const char* WIFI_PASS = "01034797777";
  g_shield.begin(WIFI_SSID, WIFI_PASS);

  g_shield.print();
}


/*
IoTMakers
*/
IoTMakers g_im;

const char deviceID[]   = "khw110D1571358056741";
const char authnRqtNo[] = "gjqr3htvh";
const char extrSysID[]  = "OPEN_TCP_001PTL001_1000007731";

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

Servo servo1;
Servo servo2;
int radian = 0;

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

  init_shield();
  
  init_iotmakers();
  servo1.attach(6);
  servo2.attach(7);
  
  servo1.write(30);
  servo2.write(150);
  delay(1000);
  servo1.write(180);
  servo2.write(0);
  delay(1000);
  servo1.write(90);
  servo2.write(90);
  delay(1000);
  servo1.write(30);
  servo2.write(150);
  delay(1000);
}

void loop(void)
{
  static unsigned long tick = millis();
  // 5초 주기로 센서 정보 송신
  
  
  // IoTMakers 서버 수신처리 및 keepalive 송신
  g_im.loop();
   
}


void mycb_numdata_handler(char *tagid, double numval)
{
  // !!! USER CODE HERE
  //Serial.print(tagid);Serial.print(F("="));Serial.println(numval);
}

void mycb_strdata_handler(char *tagid, char *strval)
{
  
  if ( strcmp(tagid, "Servo")==0 && strcmp(strval, "ONE")==0 ){   
    servo1.write(90);
    servo2.write(90);
    delay(1000);
    Serial.println("Half Open");
  }else if ( strcmp(tagid, "Servo")==0 && strcmp(strval, "OFF")==0 ){   
    servo1.write(30);
    servo2.write(150);
    Serial.println("Nomal");
    }
    
  if( strcmp(tagid, "Servo")==0 && strcmp(strval, "TWO")==0 ){   
    servo1.write(180);
    servo2.write(0);
    delay(1000);
    Serial.println("Full Open");
  }else if ( strcmp(tagid, "Servo")==0 && strcmp(strval, "OFF")==0 ){   
    servo1.write(30);
    servo2.write(150);
    delay(1000);
    Serial.println("Nomal");
    }
 
}

void mycb_resp_handler(long long trxid, char *respCode)
{
  if ( strcmp(respCode, "100")==0 )
    Serial.println("resp:OK");
  else
    Serial.println("resp:Not OK");
}
