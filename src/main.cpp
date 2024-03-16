#include "TFT_eSPI.h" // Hardware-specific library
#include "WebSocketsClient.h"
#include "env.hpp"
#include "utils/mempool.hpp"
#include <Arduino.h>
#include <SPI.h> // library for SPI communication
#include <WiFi.h>

#define SERIALDEBUG

/// Display ///
TFT_eSPI display = TFT_eSPI(); // Invoke custom library

/// WebSocket ///
String wsMsg;
bool newMsg = false;
WebSocketsClient webSocketClient;

void webSocketEvent(WStype_t type, uint8_t *strload, size_t length);

void setup(void) {
#ifdef SERIALDEBUG
    Serial.begin(9600);
    Serial.printf("Starting...\n");
#endif

    /// Init WiFi ///
    WiFi.begin(ENV_SSID, ENV_PASS);
#ifdef SERIALDEBUG
    Serial.printf("Connecting to WiFi");
#endif
    while (WiFi.status() != WL_CONNECTED) {
#ifdef SERIALDEBUG
        Serial.printf(".");
#endif
        delay(250);
    }
#ifdef SERIALDEBUG
    Serial.printf("\nConnected to the WiFi network\n");
#endif

    /// Init WebSocket ///
    webSocketClient.beginSSL("mempool.space", 443, "/api/v1/ws");
    webSocketClient.onEvent(webSocketEvent);
    webSocketClient.setReconnectInterval(1000);
    while (!webSocketClient.isConnected()) {
#ifdef SERIALDEBUG
        Serial.print(".");
#endif
        webSocketClient.loop();
    }
#ifdef SERIALDEBUG
    Serial.printf("Conceted to websocket!\n");
#endif

    display.init();

    display.fillScreen(TFT_BLACK);
    display.drawRect(0, 0, display.width(), display.height(), TFT_GREEN);

    // Set "cursor" at top left corner of display (0,0) and select font 4
    display.setCursor(0, 4, 4);

    // Set the font colour to be white with a black background
    display.setTextColor(TFT_WHITE);

    // We can now plot text on screen using the "print" class
    display.println(" Initialised default\n");
    display.println(" White text");

    display.setTextColor(TFT_RED);
    display.println(" Red text");

    display.setTextColor(TFT_GREEN);
    display.println(" Green text");

    display.setTextColor(TFT_BLUE);
    display.println(" Blue text");
}

void loop() {
    if (newMsg) {
        // Serial.printf("Received: %s\n", wsMsg.c_str());
        lastBlock(wsMsg);
        newMsg = false;
    } else {
        webSocketClient.loop();
    }

    // display.setRotation(3);
    // display.invertDisplay(false); // Where i is true or false

    // display.fillScreen(TFT_BLACK);
    // display.drawRect(0, 0, display.width(), display.height(), TFT_GREEN);

    // display.setCursor(0, 4, 4);

    // display.setTextColor(TFT_WHITE);
    // display.println(" Invert OFF\n");

    // display.println(" White text");

    // display.setTextColor(TFT_RED);
    // display.println(" Red text");

    // display.setTextColor(TFT_GREEN);
    // display.println(" Green text");

    // display.setTextColor(TFT_BLUE);
    // display.println(" Blue text");

    // display.setTextColor(TFT_YELLOW);
    // display.println(" Yellow text");

    // delay(5000);

    // // Binary inversion of colours
    // display.setRotation(0);
    // display.invertDisplay(true); // Where i is true or false

    // display.fillScreen(TFT_BLACK);
    // display.drawRect(0, 0, display.width(), display.height(), TFT_GREEN);

    // display.setCursor(0, 4, 4);

    // display.setTextColor(TFT_WHITE);
    // display.println(" Invert ON\n");

    // display.println(" White text");

    // display.setTextColor(TFT_RED);
    // display.println(" Red text");

    // display.setTextColor(TFT_GREEN);
    // display.println(" Green text");

    // display.setTextColor(TFT_BLUE);
    // display.println(" Blue text");

    // display.setTextColor(TFT_YELLOW);
    // display.println(" Yellow text");

    // delay(5000);
}

/*************************************************************************************************/
/*                                       WebSocket                                               */
/*************************************************************************************************/

void webSocketEvent(WStype_t type, uint8_t *strload, size_t length) {
    switch (type) {
    case WStype_DISCONNECTED:
#ifdef SERIALDEBUG
        Serial.printf("[WS] Disconnected!\n");
#endif
        break;
    case WStype_CONNECTED:
#ifdef SERIALDEBUG
        Serial.printf("\n[WS] Connected and send txt\n");
#endif
        webSocketClient.sendTXT("{\"action\":\"want\",\"data\":[\"blocks\"]}");
        break;
    case WStype_TEXT:
        wsMsg = (char *)strload;
#ifdef SERIALDEBUG
        Serial.printf("\n[WS] Received data from socket\n");
#endif
        newMsg = true;
        break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
        break;
    }
}