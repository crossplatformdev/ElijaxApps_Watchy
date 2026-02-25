/**
 * HelloWorldApp.cpp
 * 
 * Simple example demonstrating the Watchy UiSDK.
 * 
 * Features:
 *   - Displays "Hello World" text
 *   - Shows current time as "hh:mm:ss"
 *   - Demonstrates theme-aware rendering
 *   - Uses button input to exit
 * 
 * License: GPLv3
 * Author: Elías A. Angulo Klein (crossplatformdev)
 */

#include "../../src/sdk/UiSDK.h"
#include "../../src/watchy/Watchy.h"
#include "../../src/sdk/Fonts.h"

class HelloWorldApp : public App {
public:
    void run(Watchy &watchy) override {
        showHelloWorld(watchy);
    }

private:
    void showHelloWorld(Watchy &watchy) {
        auto &display = watchy.display;
        
        // Get current time
        tmElements_t &tm = watchy.currentTime;
        
        // Format time string as "hh:mm:ss"
        char timeStr[12];
        sprintf(timeStr, "%02d:%02d:%02d", tm.Hour, tm.Minute, tm.Second);
        
        // Get theme colors (respects dark/light mode setting)
        uint16_t bg = gDarkMode ? GxEPD_BLACK : GxEPD_WHITE;
        uint16_t fg = gDarkMode ? GxEPD_WHITE : GxEPD_BLACK;
        
        // Setup display for full-screen drawing
        UiSDK::setFullWindow(display);
        UiSDK::fillScreen(display, bg);
        
        // Draw "Hello World" text
        UiSDK::setFont(display, &FreeMonoBold18pt7b);
        UiSDK::setTextColor(display, fg);
        
        // Center "Hello World" horizontally
        int16_t x1, y1;
        uint16_t w, h;
        UiSDK::getTextBounds(display, "Hello World", 0, 0, &x1, &y1, &w, &h);
        int16_t helloX = (200 - w) / 2;  // Center on 200px wide screen
        int16_t helloY = 80;
        
        UiSDK::setCursor(display, helloX, helloY);
        UiSDK::print(display, "Hello World");
        
        // Draw current time label
        UiSDK::setFont(display, &FreeMonoBold9pt7b);
        UiSDK::setCursor(display, 10, 120);
        UiSDK::print(display, "The current time is:");
        
        // Draw time value (larger font)
        UiSDK::setFont(display, &FreeMonoBold12pt7b);
        
        // Center time horizontally
        UiSDK::getTextBounds(display, timeStr, 0, 0, &x1, &y1, &w, &h);
        int16_t timeX = (200 - w) / 2;
        int16_t timeY = 150;
        
        UiSDK::setCursor(display, timeX, timeY);
        UiSDK::print(display, timeStr);
        
        // Draw exit hint
        UiSDK::setFont(display, &FreeMono9pt7b);
        UiSDK::setCursor(display, 30, 190);
        UiSDK::print(display, "MENU = Exit");
        
        // Push to display
        UiSDK::displayUpdate(display);
        
        // Wait for MENU button press to exit
        waitForExit();
    }
    
    void waitForExit() {
        // Wait until MENU button is released (if held from previous screen)
        while (digitalRead(MENU_BTN_PIN) == LOW) {
            delay(10);
        }
        
        // Wait for MENU button press
        while (true) {
            if (UiSDK::buttonPressed(MENU_BTN_PIN)) {
                break;
            }
            delay(50);
        }
    }
};

// Singleton instance (used by app registry)
static HelloWorldApp g_helloWorldApp;

App *getHelloWorldApp() {
    return &g_helloWorldApp;
}
