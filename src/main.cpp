#include "TFT_eSPI.h"
#include "WebSocketsClient.h"
#include "env.hpp"
#include "utils/mempool.hpp"
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>

#define SERIALDEBUG
#define WS_MEMPOOL
#define WS_COINBASE

/// Display ///
/* If you use ESP32-Sx + external OLed to see pins to connect Oled go to
 * .pio/libdeps/nodemcu-32s/TFT_eSPI/User_Setups/Setup25_TTGO_T_Display.h
 */
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite display = TFT_eSprite(&tft);

uint8_t lineWidth = 4;

/// WebSocket ///
TaskHandle_t webSocketsHandle;
void taskWebSocketsHandle(void *vpParameters);

/// Mempool
#ifdef WS_MEMPOOL
String wsMempoolMsg;
bool newMempoolMsg = false;
WebSocketsClient webSocketMempoolClient; // declare instance of websocket

void webSocketMempoolEvent(WStype_t type, uint8_t *strload, size_t length);
#endif

/// Coinbase
#ifdef WS_COINBASE
String wsCoinbaseMsg;
bool newCoinbaseMsg = false;
WebSocketsClient webSocketCoinbaseClient; // declare instance of websocket

void webSocketCoinbaseEvent(WStype_t type, uint8_t *strload, size_t length);
#endif

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

/// Init WebSockets ///
#ifdef WS_MEMPOOL
    webSocketMempoolClient.beginSSL("mempool.space", 443, "/api/v1/ws");
    webSocketMempoolClient.onEvent(webSocketMempoolEvent);
    webSocketMempoolClient.setReconnectInterval(1000);
#endif
#ifdef WS_COINBASE
    webSocketCoinbaseClient.beginSSL("ws-feed.exchange.coinbase.com", 443);
    webSocketCoinbaseClient.onEvent(webSocketCoinbaseEvent);
    webSocketCoinbaseClient.setReconnectInterval(1000);
#endif

    xTaskCreatePinnedToCore(taskWebSocketsHandle, "Web Sockets Handle", 10000, NULL, 0, &webSocketsHandle, 0);

    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLUE);
    delay(2500);
    display.deleteSprite();
    display.createSprite(tft.width(), tft.height());

    display.fillSprite(TFT_BLACK);
    display.fillRect(0, 0, display.width(), lineWidth, TFT_GREEN);                            // Top
    display.fillRect(0, display.height() - lineWidth, display.width(), lineWidth, TFT_GREEN); // Bottom
    display.fillRect(0, 0, lineWidth, display.height(), TFT_GREEN);                           // Left
    display.fillRect(display.width() - lineWidth, 0, lineWidth, display.height(), TFT_GREEN); // Right

    display.setTextColor(TFT_WHITE);
    display.drawString("Initialised", lineWidth * 2, 32, 4);
    display.drawString("White text", lineWidth * 2, 64, 4);

    display.setTextColor(TFT_RED);
    display.drawString("Red text", lineWidth * 2, 96, 4);

    display.setTextColor(TFT_GREEN);
    display.drawString("Green text", lineWidth * 2, 128, 4);

    display.setTextColor(TFT_BLUE);
    display.drawString("Blue text", lineWidth * 2, 160, 4);

    display.pushSprite(0, 0);

    delay(5000);

    display.deleteSprite();
    display.createSprite(tft.width() - lineWidth * 2, tft.height() - lineWidth * 2);
    display.fillSprite(TFT_WHITE);
    display.setTextColor(TFT_BLACK);
    display.drawString("To setup", 4, 32, 4);
    display.pushSprite(lineWidth, lineWidth);
}

String lastBlock = "";
String price = "";
String auxLastBlock = "";
String auxPrice = "";
bool newMsgs = false;

void loop() {
#ifdef SERIALDEBUG
    // Serial.printf("Looping...\n");
#endif

#ifndef WS_MEMPOOL
    newMsgs = newCoinbaseMsg;
#endif
#ifndef WS_COINBASE
    newMsgs = newMempoolMsg;
#endif
#ifdef WS_MEMPOOL
#ifdef WS_COINBASE
    newMsgs = newMempoolMsg || newCoinbaseMsg;
#endif
#endif

    if (newMsgs) {
        Serial.printf("Loop running on core %d\n", xPortGetCoreID());

        display.deleteSprite();
        display.createSprite(tft.width() - lineWidth * 2, tft.height() - lineWidth * 2);
        display.fillSprite(TFT_BLACK);

        display.setTextColor(TFT_ORANGE);
        display.drawString("BITCOIN", 4, 4, 4);

#ifdef WS_MEMPOOL
        display.setTextColor(TFT_WHITE);
        display.drawString("Last block:", 4, 32, 4);
        display.setTextColor(TFT_ORANGE);
        lastBlock = newMempoolMsg && (auxLastBlock = getLastBlock(wsMempoolMsg)).isEmpty() ? auxLastBlock : lastBlock;
#ifdef SERIALDEBUG
        Serial.printf("=> lastBlock: %s\n", lastBlock.c_str());
#endif
        display.drawString(lastBlock, 4, 64, 4);
        newMempoolMsg = false;
#endif

#ifdef WS_COINBASE
        display.setTextColor(TFT_WHITE);
        display.drawString("Price:", 4, 96, 4);
        display.setTextColor(TFT_ORANGE);
        price = newCoinbaseMsg && (auxPrice = getBTCPrice(wsCoinbaseMsg)).isEmpty() ? "$" + auxPrice : price;
#ifdef SERIALDEBUG
        Serial.printf("=> price: %s\n", price.c_str());
#endif
        display.drawString(price, 4, 128, 4);
        newCoinbaseMsg = false;
#endif

        display.pushSprite(lineWidth, lineWidth);
    }
}

/*************************************************************************************************/
/*                                       WebSocket                                               */
/*************************************************************************************************/

/// Mempool
#ifdef WS_MEMPOOL
void webSocketMempoolEvent(WStype_t type, uint8_t *strload, size_t length) {
    switch (type) {
    case WStype_DISCONNECTED:
#ifdef SERIALDEBUG
        Serial.printf("[WS Mempool] Disconnected!\n");
#endif
        break;
    case WStype_CONNECTED:
#ifdef SERIALDEBUG
        Serial.printf("[WS Mempool] Connected\n");
#endif
        webSocketMempoolClient.sendTXT(
            "{\"action\":\"want\",\"data\":[\"blocks\"]}"); // send message to server when connected
        break;
    case WStype_TEXT:
#ifdef SERIALDEBUG
        Serial.printf("[WS Mempool] Received data from socket\n");
#endif
        wsMempoolMsg = (char *)strload;
        newMempoolMsg = true;
        break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
        break;
    }
}

void taskWebSocketsHandle(void *vpParameters) {
#ifdef SERIALDEBUG
    Serial.printf("Start Task WebSocket Mempool Handle\n");
    Serial.printf("taskWebSocketsHandle running on core %d\n", xPortGetCoreID());
#endif

    for (;;) {
#ifdef WS_MEMPOOL
        webSocketMempoolClient.loop();
#endif
#ifdef WS_COINBASE
        webSocketCoinbaseClient.loop();
#endif
    }
}
#endif

/// Coinbase
#ifdef WS_COINBASE
void webSocketCoinbaseEvent(WStype_t type, uint8_t *strload, size_t length) {
    switch (type) {
    case WStype_DISCONNECTED:
#ifdef SERIALDEBUG
        Serial.printf("[WS Coinbase] Disconnected!\n");
#endif
        break;
    case WStype_CONNECTED:
#ifdef SERIALDEBUG
        Serial.printf("[WS Coinbase] Connected\n");
#endif
        webSocketCoinbaseClient.sendTXT(
            "{\"type\":\"subscribe\",\"product_ids\":[\"BTC-USD\"],\"channels\":[\"ticker_batch\"]}"); // send message
                                                                                                       // to server when
                                                                                                       // connected
        break;
    case WStype_TEXT:
#ifdef SERIALDEBUG
        Serial.printf("[WS Coinbase] Received data from socket\n");
#endif
        wsCoinbaseMsg = (char *)strload;
        newCoinbaseMsg = true;
        break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
        break;
    }
}
#endif