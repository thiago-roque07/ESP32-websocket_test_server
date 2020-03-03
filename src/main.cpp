/*
 * WebSocketServer.ino
 *
 *  Created on: 22.05.2015
 *
 */

#include <Arduino.h>

#include <WiFi.h>
//#include <WiFiClientSecure.h>
#include <WebSocketsServer.h>

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <math.h>
#include "filter.h"

#ifndef APSSID
#define APSSID "ESP-EEG_AP1"
#define APPSK  "12345678"
#endif

#define measBufLength 16

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

uint16_t isClient = 0;
float new_measure = -5000;
float old_measure = 10;
uint8_t *payload_buff;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time data was updated

// Updates data readings every 4ms
const long interval = 4;

float buf1_n = 0;
float buf2_n = 0;
float x_in_n;

float buf1_hp = 0;
float buf2_hp = 0;
float x_in_hp;
float masterValue;
float measBuf [16];
int16_t bufIdx = 0;
bool bufFull = 0;

Adafruit_ADS1115 ads(0x49);  /* Use this for the 16-bit version */

WebSocketsServer webSocket = WebSocketsServer(81);

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
	const uint8_t* src = (const uint8_t*) mem;
	Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) {
		if(i % cols == 0) {
			Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		}
		Serial.printf("%02X ", *src);
		src++;
	}
	Serial.printf("\n");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            isClient--;
            break;
        case WStype_CONNECTED:
            {
              IPAddress ip = webSocket.remoteIP(num);
              Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
              isClient++;
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary length: %u\n", num, length);
            hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
		case WStype_ERROR:
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
    }

}

void setup() {
	delay(1000);

	ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.begin();
	Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

	Serial.print("Configuring access point...");
	/* You can remove the password parameter if you want the AP to be open. */
	WiFi.softAP(ssid, password);

	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
	float EEG_value;
	float EEG_notch_value;
	float EEG_HP_notch_value;
  webSocket.loop();

  unsigned long currentMillis = millis();

	if (currentMillis - previousMillis >= interval) {
  	previousMillis = currentMillis;

    EEG_value = readMeasure();
    EEG_notch_value = filterNotch(EEG_value);
    EEG_HP_notch_value = filterHighPass(EEG_notch_value);

    // measBuf[bufIdx] = EEG_HP_notch_value;
    masterValue = EEG_HP_notch_value;

    //Serial.println(masterValue);

    // if(bufIdx == measBufLength){
    //   //snedbuffer
    //   bufIdx = 0;
    //   bufFull = 1;
    //   Serial.println("Buffer Full");
    // }
		//new_measure = masterValue;
		new_measure++;
 }


    if(isClient) {
      if(new_measure != old_measure){
        payload_buff = (uint8_t*) &new_measure;
        webSocket.broadcastBIN(payload_buff, sizeof(new_measure));
      }
    }
		else {
			Serial.printf("connection lost");
		}
    old_measure = new_measure;
}

float readMeasure()
{
  return (float) ads.readADC_SingleEnded(0);
}

float filterNotch(float x_in_n)
{
  float input_acc_n;
  float output_acc_n;

  input_acc_n = x_in_n-(a1_n*buf1_n)-(a2_n*buf2_n);
  output_acc_n = input_acc_n*b0_n+(b1_n*buf1_n)+(b2_n*buf2_n);
  buf2_n = buf1_n;
  buf1_n = input_acc_n;
  return output_acc_n;
}

float filterHighPass(float x_in_hp)
{
  float input_acc_hp;
  float output_acc_hp;

  input_acc_hp = x_in_hp-(a1_hp*buf1_hp)-(a2_hp*buf2_hp);
  output_acc_hp = input_acc_hp*b0_hp+(b1_hp*buf1_hp)+(b2_hp*buf2_hp);
  buf2_hp = buf1_hp;
  buf1_hp = input_acc_hp;
  return output_acc_hp;
}
