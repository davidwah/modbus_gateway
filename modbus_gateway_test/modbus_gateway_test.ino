#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

const char* ssid = "frank1";
const char* password = "annarieannarie";

// Start a TCP Server on port 502

WiFiServer server(502);
WiFiClient client ;

const long interval = 300;           // interval (milliseconds)
size_t len = 50;
uint8_t sbuf[100];
uint8_t rtu_buf[100];
uint8_t rtu_len;

uint16_t calcCRC(uint8_t u8_buff_size)
{
  uint32_t tmp, tmp2, flag;
  tmp = 0xFFFF;

  for (uint8_t i = 0; i < u8_buff_size; i++)
  {
    tmp = tmp ^ rtu_buf[i];
      for (uint8_t j = 1; j <= 8; j++)
    {
      flag = tmp & 0x0001;
      tmp >>= 1;

      if (flag)
        tmp ^= 0xA001;
    }
  }
  tmp2 = tmp >> 8;
  tmp = (tmp << 8) | tmp2;
  tmp &= 0xFFFF;

  return tmp;
}
SoftwareSerial swSer(D5, D6, false, 256); //tx,rx   d5,d6 d0-re

void setup() {
  swSer.begin(9600);
  Serial.begin(115200);
  WiFi.begin(ssid,password);
  Serial.println("");
  //Wait for connection
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to "); Serial.println(ssid);
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());
 
  // Start the TCP server
  server.begin();
  pinMode(D0,OUTPUT);
}

void loop() {
      if (!client.connected()) {
      client = server.available();
    }
    else{     
    
    if (client.available() > 0) {
      
      int readnum = client.read(sbuf,client.available());
      for (int i=0 ;i<readnum;i++){
      Serial.print(sbuf[i],HEX);
      }
      Serial.println();
      rtu_len = sbuf[5];
      for (int i=0 ;i<readnum-6;i++){
        rtu_buf[i] = sbuf[i+6];
      }
      int value = calcCRC(rtu_len); 
      rtu_buf[rtu_len]  = (value & 0xFF00) >> 8;
      rtu_buf[rtu_len+1] = value & 0x00FF;
      for (int i=0 ;i<rtu_len+2;i++){
      Serial.print(rtu_buf[i],HEX);
      }
      Serial.println();
      swSer.flush();
      digitalWrite(D0,HIGH);
      swSer.write(rtu_buf,rtu_len + 2);
      digitalWrite(D0,LOW);
      memset(rtu_buf, 0, 50);
      unsigned long currentMillis = millis();
      while(!swSer.available() or (millis() - currentMillis  <= interval)) {
        yield();
        }
        int i = 6;
        while (swSer.available() > 0) {
          rtu_buf[i] = swSer.read();
          i ++;
          yield();
           }
           rtu_len = i- 2;
           rtu_buf[5] = rtu_len -6 ;
           rtu_buf[0] = sbuf[0];
           rtu_buf[1] = sbuf[1];
           client.write((const uint8_t*)rtu_buf,rtu_len); 
           //Unfortunately there is a bad function overload in the library,
           //but you can just typecast the clientbuf to overcome this problem.....
           //   eg ... client.write((const uint8_t*)clientBuf, sizeof(clientBuf));
  }
 }
}
