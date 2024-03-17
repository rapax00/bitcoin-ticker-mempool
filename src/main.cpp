#include "TFT_eSPI.h"
#include "env.hpp"
#include "utils/mempool.hpp"
#include <Arduino.h>
#include <HTTPClient.h>
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

HTTPClient httpClient;
uint16_t httpCode;

String lastBlock;
String price;
String aux;

void loop() {
#ifdef SERIALDEBUG
    Serial.printf("Looping...\n");
#endif

    // Get last block
    String httpPayloadBlock;

    httpClient.begin("https://mempool.space/api/v1/blocks");
    httpCode = httpClient.GET();

    if (httpCode >= 200 && httpCode < 300) {
        httpPayloadBlock = httpClient.getString();
    }
#ifdef SERIALDEBUG
    else {
        Serial.printf("[HTTP] GET 'blocks' failed, error: %s\n", httpClient.errorToString(httpCode).c_str());
    }
#endif

    httpClient.end(); // Close connection

    // Get BTC price
    String httpPayloadPrice;

    httpClient.begin("https://mempool.space/api/v1/prices");
    httpCode = httpClient.GET();

    if (httpCode >= 200 && httpCode < 300) {
        httpPayloadPrice = httpClient.getString();
    }
#ifdef SERIALDEBUG
    else {
        Serial.printf("[HTTP] GET 'prices' failed, error: %s\n", httpClient.errorToString(httpCode).c_str());
    }
#endif

    httpClient.end(); // Close connection

    display.deleteSprite();
    display.createSprite(tft.width() - lineWidth * 2, tft.height() - lineWidth * 2);
    display.fillSprite(TFT_BLACK);

    display.setTextColor(TFT_ORANGE);
    display.drawString("BITCOIN", 4, 4, 4);

    display.setTextColor(TFT_WHITE);
    display.drawString("Last block:", 4, 32, 4);
    display.setTextColor(TFT_ORANGE);
    lastBlock = (aux = getLastBlock(httpPayloadBlock)) ? aux : lastBlock;
    display.drawString(lastBlock, 4, 64, 4);

    display.setTextColor(TFT_WHITE);
    display.drawString("Price:", 4, 96, 4);
    display.setTextColor(TFT_ORANGE);
    String price = (aux = getBTCPrice(httpPayloadPrice)) ? "$" + aux : price;
    display.drawString(price, 4, 128, 4);

    display.pushSprite(lineWidth, lineWidth);

    delay(5000);
}