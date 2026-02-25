#include "Watchy_Mario.h"
#include "./../../sdk/UiSDK.h"
#define NUM_W 44
#define NUM_H 44
#define COIN_W 24
#define COIN_H 30
#define PIPE_W 42
#define PIPE_H 47
#define MARIO_W 56
#define MARIO_H 54
#define NUM_SPACING 4
#define COIN_SPACING 4
#define FLOOR_H 19
#define PIPE_PADDING DISPLAY_HEIGHT - FLOOR_H - PIPE_H
#define X_PADDING (DISPLAY_WIDTH - (4*NUM_W) - (3*NUM_SPACING))/2
#define Y_PADDING 2*COIN_SPACING+COIN_H


static const unsigned char *numbers [10] = {mario0, mario1, mario2, mario3, mario4, mario5, mario6, mario7, mario8, mario9};

void WatchyMario::drawWatchFace(){
    // Follow global theme background
    UiSDK::fillScreen(display, UiSDK::getWatchfaceBg(BASE_POLARITY));  
    UiSDK::setTextColor(display, UiSDK::getWatchfaceFg(BASE_POLARITY));


    UIImageSpec images[10]{};

    // Background
    images[0].bitmap          = mariobg;
    images[0].x               = 0;
    images[0].y               = 0;
    images[0].w               = DISPLAY_WIDTH;
    images[0].h               = DISPLAY_HEIGHT;
    images[0].fromProgmem     = true;
    images[0].fillBackground  = false;

    int hour10   = currentTime.Hour/10;
    int hour01   = currentTime.Hour%10;
    int minute10 = currentTime.Minute/10;
    int minute01 = currentTime.Minute%10;

    int pos = 0;
    if(hour01 == 0 && minute10 == 0 && minute01 == 0){
        pos = 0;
    }
    else if(minute10 == 0 && minute01 == 0){
        pos = 1;
    }  
    else if(minute01 == 0){
        pos = 2;
    }else{
        pos = 3;
    }

    // Mario sprite
    images[1].bitmap          = (pos < 2 ? mariomariol : mariomarior);
    images[1].x               = X_PADDING + pos*(NUM_SPACING + NUM_W) + (NUM_W/2 - MARIO_W/2) + (pos < 2 ? 8 : -8);
    images[1].y               = Y_PADDING+NUM_H + 4;
    images[1].w               = MARIO_W;
    images[1].h               = MARIO_H;
    images[1].fromProgmem     = true;
    images[1].fillBackground  = false;

    // Coin
    images[2].bitmap          = mariocoin;
    images[2].x               = X_PADDING + pos*(NUM_SPACING + NUM_W) + (NUM_W/2 - COIN_W/2);
    images[2].y               = COIN_SPACING;
    images[2].w               = COIN_W;
    images[2].h               = COIN_H;
    images[2].fromProgmem     = true;
    images[2].fillBackground  = false;

    // Pipes
    uint8_t idx = 3;
    if(pos == 0){
        images[idx].bitmap          = mariopipe;
        images[idx].x               = DISPLAY_WIDTH - 2*PIPE_W;
        images[idx].y               = PIPE_PADDING;
        images[idx].w               = PIPE_W;
        images[idx].h               = PIPE_H;
        images[idx].fromProgmem     = true;
        images[idx].fillBackground  = false;
        idx++;
    } else if(pos == 1 || pos == 2){
        images[idx].bitmap          = mariopipe;
        images[idx].x               = X_PADDING;
        images[idx].y               = PIPE_PADDING;
        images[idx].w               = PIPE_W;
        images[idx].h               = PIPE_H;
        images[idx].fromProgmem     = true;
        images[idx].fillBackground  = false;
        idx++;

        images[idx].bitmap          = mariopipe;
        images[idx].x               = DISPLAY_WIDTH - PIPE_W - X_PADDING;
        images[idx].y               = PIPE_PADDING;
        images[idx].w               = PIPE_W;
        images[idx].h               = PIPE_H;
        images[idx].fromProgmem     = true;
        images[idx].fillBackground  = false;
        idx++;
    } else {
        images[idx].bitmap          = mariopipe;
        images[idx].x               = 2*PIPE_W;
        images[idx].y               = PIPE_PADDING;
        images[idx].w               = PIPE_W;
        images[idx].h               = PIPE_H;
        images[idx].fromProgmem     = true;
        images[idx].fillBackground  = false;
        idx++;
    }

    // Hour digits
    images[idx].bitmap          = numbers[hour10];
    images[idx].x               = X_PADDING;
    images[idx].y               = (pos == 0 ? Y_PADDING : Y_PADDING + 20);
    images[idx].w               = NUM_W;
    images[idx].h               = NUM_H;
    images[idx].fromProgmem     = true;
    images[idx].fillBackground  = false;
    idx++;

    images[idx].bitmap          = numbers[hour01];
    images[idx].x               = X_PADDING+NUM_SPACING+NUM_W;
    images[idx].y               = (pos == 1 ? Y_PADDING : Y_PADDING + 20);
    images[idx].w               = NUM_W;
    images[idx].h               = NUM_H;
    images[idx].fromProgmem     = true;
    images[idx].fillBackground  = false;
    idx++;

    // Minute digits
    images[idx].bitmap          = numbers[minute10];
    images[idx].x               = X_PADDING+2*(NUM_SPACING+NUM_W);
    images[idx].y               = (pos == 2 ? Y_PADDING : Y_PADDING + 20);
    images[idx].w               = NUM_W;
    images[idx].h               = NUM_H;
    images[idx].fromProgmem     = true;
    images[idx].fillBackground  = false;
    idx++;

    images[idx].bitmap          = numbers[minute01];
    images[idx].x               = X_PADDING+3*(NUM_SPACING+NUM_W);
    images[idx].y               = (pos == 3 ? Y_PADDING : Y_PADDING + 20);
    images[idx].w               = NUM_W;
    images[idx].h               = NUM_H;
    images[idx].fromProgmem     = true;
    images[idx].fillBackground  = false;

    UIAppSpec app{};
    app.images        = images;
    app.imageCount    = idx + 1;
    app.texts         = nullptr;
    app.textCount     = 0;
    app.menus         = nullptr;
    app.menuCount     = 0;
    app.buttons       = nullptr;
    app.buttonCount   = 0;
    app.checkboxes    = nullptr;
    app.checkboxCount = 0;
    app.scrollTexts   = nullptr;
    app.scrollTextCount = 0;
    app.callbacks     = nullptr;
    app.callbackCount = 0;

    UiSDK::renderApp(*this, app);
}

void showWatchFace_Mario(Watchy &watchy) {
    WatchyMario face(watchy.settings);
    face.currentTime = watchy.currentTime;
    face.drawWatchFace();
}