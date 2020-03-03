/*
 * WebSocketServer.ino
 *
 *  Created on: 22.05.2015
 *
 */

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <WebSocketsServer.h>

uint16_t isClient = 0;
float new_measure = 22.5;
float old_measure = 10;
uint8_t *payload_buff; 

WiFiMulti WiFiMulti;
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

    Serial.begin(115200);

    //Serial.setDebugOutput(true);

    Serial.println();
    Serial.println();
    Serial.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    WiFiMulti.addAP("SSID", "passpasspass");

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    webSocket.loop();

    // if(new_timer  - old_timer> interval )
    // {
    //   readSensor();
    //   filter1();
    //   new_measure = filter2();
    // }
    

    if(isClient) {
      if(new_measure != old_measure){
        payload_buff = (uint8_t*) &new_measure;
        webSocket.broadcastBIN(payload_buff, sizeof(new_measure)); 
      } 
    }
    old_measure = new_measure;
}