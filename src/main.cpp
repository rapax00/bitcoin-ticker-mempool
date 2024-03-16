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
TFT_eSPI display = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&display);

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

    spr.init();
    spr.setRotation(0);
    spr.createSprite(spr.width(), spr.height());

    spr.fillScreen(TFT_BLACK);
    spr.drawRect(0, 0, spr.width(), spr.height(), TFT_GREEN);

    // Set "cursor" at top left corner of display (0,0) and select font 4
    spr.setCursor(0, 4, 4);

    // Set the font colour to be white with a black background
    spr.setTextColor(TFT_WHITE);

    // We can now plot text on screen using the "print" class
    spr.println(" Initialised default\n");
    spr.println(" White text");

    spr.setTextColor(TFT_RED);
    spr.println(" Red text");

    spr.setTextColor(TFT_GREEN);
    spr.println(" Green text");

    spr.setTextColor(TFT_BLUE);
    spr.println(" Blue text");

    delay(1000);
}

HTTPClient httpClient;
int httpCode;

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

    spr.fillScreen(TFT_BLACK);
    spr.drawRect(0, 0, spr.width(), spr.height(), TFT_GREEN);

    spr.setTextColor(TFT_ORANGE);
    spr.setCursor(16, 16, 4);
    spr.printf("BITCOIN\n");

    spr.setTextColor(TFT_WHITE);
    spr.printf("\n Last block:\n");
    spr.setTextColor(TFT_ORANGE);
    spr.printf(" %d\n", getLastBlock(httpPayloadBlock));

    spr.setTextColor(TFT_WHITE);
    spr.printf("\n BTC price:\n");
    spr.setTextColor(TFT_ORANGE);
    spr.printf(" $%d", getBTCPrice(httpPayloadPrice));

    delay(5000);
}