#include "TFT_eSPI.h"
#include <Arduino.h>
#include <SPI.h> // library for SPI communication

/// Display ///
TFT_eSPI display = TFT_eSPI(); // Invoke custom library

void setup(void) {
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

    delay(5000);
}

void loop() {
    display.setRotation(3);
    display.invertDisplay(false); // Where i is true or false

    display.fillScreen(TFT_BLACK);
    display.drawRect(0, 0, display.width(), display.height(), TFT_GREEN);

    display.setCursor(0, 4, 4);

    display.setTextColor(TFT_WHITE);
    display.println(" Invert OFF\n");

    display.println(" White text");

    display.setTextColor(TFT_RED);
    display.println(" Red text");

    display.setTextColor(TFT_GREEN);
    display.println(" Green text");

    display.setTextColor(TFT_BLUE);
    display.println(" Blue text");

    display.setTextColor(TFT_YELLOW);
    display.println(" Yellow text");

    delay(5000);

    // Binary inversion of colours
    display.setRotation(0);
    display.invertDisplay(true); // Where i is true or false

    display.fillScreen(TFT_BLACK);
    display.drawRect(0, 0, display.width(), display.height(), TFT_GREEN);

    display.setCursor(0, 4, 4);

    display.setTextColor(TFT_WHITE);
    display.println(" Invert ON\n");

    display.println(" White text");

    display.setTextColor(TFT_RED);
    display.println(" Red text");

    display.setTextColor(TFT_GREEN);
    display.println(" Green text");

    display.setTextColor(TFT_BLUE);
    display.println(" Blue text");

    display.setTextColor(TFT_YELLOW);
    display.println(" Yellow text");

    delay(5000);
}