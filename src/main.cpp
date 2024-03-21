#include "TFT_eSPI.h"
#include "WebSocketsClient.h"
#include "env.hpp"
#include "utils/mempool.hpp"
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>

#define SERIALDEBUG

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

String wsMsg;
bool newMsg = false;
WebSocketsClient webSocketClient; // declare instance of websocket

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

    /// Init WebSockets ///
    webSocketClient.begin(ENV_WS_SERVER, 8080);
    webSocketClient.onEvent(webSocketEvent);
    webSocketClient.setReconnectInterval(1000);

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
bool pusheado = false;

void loop() {
#ifdef SERIALDEBUG
    // Serial.printf("Looping...\n");
#endif

    if (newMsg) {
        Serial.printf("Loop running on core %d\n", xPortGetCoreID());

        display.deleteSprite();
        display.createSprite(tft.width() - lineWidth * 2, tft.height() - lineWidth * 2);
        display.fillSprite(TFT_BLACK);

        display.setTextColor(TFT_ORANGE);
        display.drawString("BITCOIN", 4, 4, 4);

        display.setTextColor(TFT_WHITE);
        display.drawString("Last block:", 4, 32, 4);
        display.setTextColor(TFT_ORANGE);
        lastBlock = newMsg && !((auxLastBlock = getLastBlock(wsMsg)).isEmpty()) ? auxLastBlock : lastBlock;
#ifdef SERIALDEBUG
        Serial.printf("=> lastBlock: %s\n", lastBlock.c_str());
#endif
        display.drawString(lastBlock, 4, 64, 4);

        display.setTextColor(TFT_WHITE);
        display.drawString("Price:", 4, 96, 4);
        display.setTextColor(TFT_ORANGE);
        price = newMsg && !((auxPrice = getBTCPrice(wsMsg)).isEmpty()) ? "$" + auxPrice : price;
#ifdef SERIALDEBUG
        Serial.printf("=> price: %s\n", price.c_str());
#endif
        display.drawString(price, 4, 128, 4);
        newMsg = false;

        display.pushSprite(lineWidth, lineWidth);
        pusheado = false;
    } else if (!pusheado) {
        uint8_t limit = 12;
        for (int i = 0; i < limit; i++) {
            display.deleteSprite();
            display.createSprite(((tft.width() - lineWidth * 2) / limit) * i, 8);
            display.fillSprite(TFT_RED);
            display.pushSprite(lineWidth, tft.height() - lineWidth - 8);
            delay(500);
        }
        pusheado = true;
    }
}

/*************************************************************************************************/
/*                                       WebSocket                                               */
/*************************************************************************************************/

void webSocketEvent(WStype_t type, uint8_t *strload, size_t length) {
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
        webSocketClient.sendTXT("{\"action\":\"want\"}"); // send message to server when connected
        break;
    case WStype_TEXT:
#ifdef SERIALDEBUG
        Serial.printf("[WS Mempool] Received data from socket\n");
#endif
        wsMsg = (char *)strload;
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

void taskWebSocketsHandle(void *vpParameters) {
#ifdef SERIALDEBUG
    Serial.printf("Start Task WebSocket Handle\n");
    Serial.printf("taskWebSocketsHandle running on core %d\n", xPortGetCoreID());
#endif

    for (;;) {
        webSocketClient.loop();
    }
}
