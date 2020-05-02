#include "Arduino.h"
#include <string.h>
#include "SH1106Wire.h"
#include "OLEDDisplayUi.h"
#include "Display.h"

SH1106Wire display(0x3c, 21, 16);

OLEDDisplayUi ui(&display);

int screenW = 128;
int screenH = 64;
int clockCenterX = screenW / 2;
int clockCenterY = ((screenH - 16) / 2) + 16; // top yellow part is 16 px height
// how many frames are there?
int frameCount = 1;

int overlaysCount = 1;

//char lines[3][64];
String lines[3];
void setLines(char *l1, char *l2, char *l3)
{
    char buf[64];

    if (l1 != NULL)
    {
        memset(buf, 0, 64);
        strlcpy(buf, &l1[1], 24);
        lines[0] = String(buf);
        lines[0].trim();
    }
    if (l2 != NULL)
    {
        memset(buf, 0, 64);
        strlcpy(buf, l2, 24);
        lines[1] = String(buf);
        lines[1].trim();
    }
    if (l3 != NULL)
    {
        memset(buf, 0, 64);
        strlcpy(buf, l3, 24);
        lines[2] = String(buf);
        lines[2].trim();
    }
    Serial.println(lines[0]);
    Serial.println(lines[1]);
}

void clockOverlay(OLEDDisplay *display, OLEDDisplayUiState *state)
{
}

void helloFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display->setFont(ArialMT_Plain_10);
    display->drawString(clockCenterX + x, clockCenterY + y, "Hello");
}

void longTextFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    display->clear();
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_10);
    int lw = 14;
    int ln = 1;
    y = 2;
    display->drawString(10, ln * lw + y, lines[0]);
    display->drawString(10, ++ln * lw + y, lines[1]);
}

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = {longTextFrame, helloFrame};

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = {clockOverlay};

void doDisplayUpdate(void *p)
{
    Serial.println("doDisplayUpdate running...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ui.switchToFrame(1);
    int remainingTimeBudget;
    while (true)
    {
        if ((remainingTimeBudget = ui.update()) > 0)
        {
            vTaskDelay(remainingTimeBudget / portTICK_PERIOD_MS);
        }
    }
    vTaskDelete(NULL);
}

void setupDisplay()
{
    // The ESP is capable of rendering 60fps in 80Mhz mode
    // but that won't give you much time for anything else
    // run it in 160Mhz mode or just set it to 30 fps
    ui.setTargetFPS(10);

    // Customize the active and inactive symbol
    //ui.setActiveSymbol(activeSymbol);
    //ui.setInactiveSymbol(inactiveSymbol);

    // You can change this to
    // TOP, LEFT, BOTTOM, RIGHT
    ui.setIndicatorPosition(TOP);

    // Defines where the first frame is located in the bar.
    ui.setIndicatorDirection(LEFT_RIGHT);

    //ui.disableAllIndicators();

    // You can change the transition that is used
    // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
    ui.setFrameAnimation(SLIDE_LEFT);

    // Add frames
    ui.setFrames(frames, frameCount);

    // Add overlays
    ui.setOverlays(overlays, overlaysCount);

    ui.disableAutoTransition();

    // Initialising the UI will init the display too.
    ui.init();

    xTaskCreate(&doDisplayUpdate, "doDisplayUpdate", 1024 * 5, NULL, 1, NULL);
}