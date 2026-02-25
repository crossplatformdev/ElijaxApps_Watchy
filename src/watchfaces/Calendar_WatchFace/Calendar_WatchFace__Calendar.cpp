#if defined(WATCHY_WRAPPER_INCLUDE) || defined(WATCHY_STANDALONE_WATCHFACE)
/*
Calendar
Version 1.1.0
https://github.com/uCBill/Calendar
Calendar watchface by Bill Eichner
*/

#include "../../watchy/Watchy.h" //include the Watchy library
#include "../../sdk/UiSDK.h"
#include "calendar_img.h"
#include "Teko_Regular12pt7b.h"

class CalendarWatchFace : public Watchy { //inherit and extend Watchy class
  using Watchy::Watchy;
  public:
    void drawWatchFace() { //override this method to customize how the watch face looks
      
      int16_t  x1, y1;
      uint16_t w, h;
      String textstring;
      int frame;
      int mth;
      int yr;
      int days;
      int wkday;
      int x;
      int y;
      const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
      const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);

      // Step counter managed by core Watchy

    
    // draw background
    UiSDK::initScreen(display);
    UiSDK::drawBitmap(display, 0,0, calendar_grid, 200, 200, fg);
    UiSDK::setTextColor(display, fg);
    UiSDK::setTextWrap(display, false);

    // draw Calendar
    mth = currentTime.Month;
    yr = currentTime.Year + 1970;
    UiSDK::setFont(display, &Teko_Regular12pt7b);
    textstring = monthStr(currentTime.Month);
    textstring.toUpperCase();
    textstring += " ";
    textstring += currentTime.Day;
    textstring += ", ";
    textstring += currentTime.Year + 1970;

        if (mth >= 12) {
        days = 31;
        if (yr <= 2020) {
        frame = -112;//Tue
        }
        else if (yr <= 2021) {
        frame = -84;//Wed
        }
        else if (yr <= 2022) {
        frame = -56;//Thu
        }
        else if (yr <= 2023) {
        frame = -28;//Mon
        }
        else if (yr <= 2024) {
        frame = -168;//Sun
        }
        else if (yr <= 2025) {
        frame = -140;//Mon
        }
        else if (yr <= 2026) {
        frame = -112;//Tue
        }
        else if (yr <= 2027) {
        frame = -84;//Wed
        }
        else if (yr <= 2028) {
        frame = -28;//Fri
        }
        else if (yr <= 2029) {
        frame = 1;//Sat
        }
        else if (yr <= 2030) {
        frame = -168;//Sun
        }
        else if (yr <= 2031) {
        frame = -140;//Mon
        }
        else if (yr <= 2032) {
        frame = -84;//Wed
        }
        else if (yr <= 2033) {
        frame = -56;//Thu
        }
        else if (yr <= 2034) {
        frame = -28;//Fri
        }
        else if (yr <= 2035) {
        frame = 1;//Sat
        }
        else if (yr <= 2036) {
        frame = -140;//Mon
        }
        else if (yr <= 2037) {
        frame = -112;//Tue
        }
        else if (yr <= 2038) {
        frame = -84;//Wed
        }
        else if (yr <= 2039) {
        frame = -56;//Thu
        }
        else if (yr <= 2040) {
        frame = 1;//Sat
        }
        }
        else if (mth >= 11) {
        days = 30;
        if (yr <= 2020) {
        frame = -168;//Sun
        }
        else if (yr <= 2021) {
        frame = -140;//Mon
        }
        else if (yr <= 2022) {
        frame = -112;//Tue
        }
        else if (yr <= 2023) {
        frame = -84;//Wed
        }
        else if (yr <= 2024) {
        frame = -28;//Fri
        }
        else if (yr <= 2025) {
        frame = 1;//Sat
        }
        else if (yr <= 2026) {
        frame = -168;//Sun
        }
        else if (yr <= 2027) {
        frame = -140;//Mon
        }
        else if (yr <= 2028) {
        frame = -84;//Wed
        }
        else if (yr <= 2029) {
        frame = -56;//Thu
        }
        else if (yr <= 2030) {
        frame = -28;//Fri
        }
        else if (yr <= 2031) {
        frame = 1;//Sat
        }
        else if (yr <= 2032) {
        frame = -140;//Mon
        }
        else if (yr <= 2033) {
        frame = -112;//Tue
        }
        else if (yr <= 2034) {
        frame = -84;//Wed
        }
        else if (yr <= 2035) {
        frame = -56;//Thu
        }
        else if (yr <= 2036) {
        frame = 1;//Sat
        }
        else if (yr <= 2037) {
        frame = -168;//Sun
        }
        else if (yr <= 2038) {
        frame = -140;//Mon
        }
        else if (yr <= 2039) {
        frame = -112;//Tue
        }
        else if (yr <= 2040) {
        frame = -56;//Thu
        }
        }
        else if (mth >= 10) {
        days = 31;
        if (yr <= 2020) {
        frame = -56;//Thu
        }
        else if (yr <= 2021) {
        frame = -28;//Fri
        }
        else if (yr <= 2022) {
        frame = 1;//Sat
        }
        else if (yr <= 2023) {
        frame = -168;//Sun
        }
        else if (yr <= 2024) {
        frame = -112;//Tue
        }
        else if (yr <= 2025) {
        frame = -84;//Wed
        }
        else if (yr <= 2026) {
        frame = -56;//Thu
        }
        else if (yr <= 2027) {
        frame = -28;//Fri
        }
        else if (yr <= 2028) {
        frame = -168;//Sun
        }
        else if (yr <= 2029) {
        frame = -140;//Mon
        }
        else if (yr <= 2030) {
        frame = -112;//Tue
        }
        else if (yr <= 2031) {
        frame = -84;//Wed
        }
        else if (yr <= 2032) {
        frame = -28;//Fri
        }
        else if (yr <= 2033) {
        frame = 1;//Sa
        }
        else if (yr <= 2034) {
        frame = -168;//Sun
        }
        else if (yr <= 2035) {
        frame = -140;//Mon
        }
        else if (yr <= 2036) {
        frame = -84;//Wed
        }
        else if (yr <= 2037) {
        frame = -56;//Thu
        }
        else if (yr <= 2038) {
        frame = -28;//Fri
        }
        else if (yr <= 2039) {
        frame = 1;//Sat
        }
        else if (yr <= 2040) {
        frame = -140;//Mon
        }
        }
        else if (mth >= 9) {
        days = 30;
        if (yr <= 2020) {
        frame = -112;//Tue
        }
        else if (yr <= 2021) {
        frame = -84;//Wed
        }
        else if (yr <= 2022) {
        frame = -56;//Thu
        }
        else if (yr <= 2023) {
        frame = -28;//Fri
        }
        else if (yr <= 2024) {
        frame = -168;//Sun
        }
        else if (yr <= 2025) {
        frame = -140;//Mon
        }
        else if (yr <= 2026) {
        frame = -112;//Tue
        }
        else if (yr <= 2027) {
        frame = -84;//Wed
        }
        else if (yr <= 2028) {
        frame = -28;//Fri
        }
        else if (yr <= 2029) {
        frame = 1;//Sat
        }
        else if (yr <= 2030) {
        frame = -168;//Sun
        }
        else if (yr <= 2031) {
        frame = -140;//Mon
        }
        else if (yr <= 2032) {
        frame = -84;//Wed
        }
        else if (yr <= 2033) {
        frame = -56;//Th
        }
        else if (yr <= 2034) {
        frame = -28;//Fri
        }
        else if (yr <= 2035) {
        frame = 1;//Sa
        }
        else if (yr <= 2036) {
        frame = -140;//Mon
        }
        else if (yr <= 2037) {
        frame = -112;//Tue
        }
        else if (yr <= 2038) {
        frame = -84;//Wed
        }
        else if (yr <= 2039) {
        frame = -56;//Thu
        }
        else if (yr <= 2040) {
        frame = 1;//Sa
        }
        }
        else if (mth >= 8) {
        days = 31;
        if (yr <= 2020) {
        frame = 1;//Sat
        }
        else if (yr <= 2021) {
        frame = -168;//Sun
        }
        else if (yr <= 2022) {
        frame = -140;//Mon
        }
        else if (yr <= 2023) {
        frame = -112;//Tue
        }
        else if (yr <= 2024) {
        frame = -56;//Thu
        }
        else if (yr <= 2025) {
        frame = -28;//Fri
        }
        else if (yr <= 2026) {
        frame = 1;//Sat
        }
        else if (yr <= 2027) {
        frame = -168;//Sun
        }
        else if (yr <= 2028) {
        frame = -112;//Tue
        }
        else if (yr <= 2029) {
        frame = -84;//Wed
        }
        else if (yr <= 2030) {
        frame = -56;//Thu
        }
        else if (yr <= 2031) {
        frame = -28;//Fri
        }
        else if (yr <= 2032) {
        frame = -168;//Sun
        }
        else if (yr <= 2033) {
        frame = -140;//Mon
        }
        else if (yr <= 2034) {
        frame = -112;//Tue
        }
        else if (yr <= 2035) {
        frame = -84;//Wed
        }
        else if (yr <= 2036) {
        frame = -28;//Fri
        }
        else if (yr <= 2037) {
        frame = 1;//Sat
        }
        else if (yr <= 2038) {
        frame = -168;//Sun
        }
        else if (yr <= 2039) {
        frame = -140;//Mon
        }
        else if (yr <= 2040) {
        frame = -84;//Wed
        }
        }
        else if (mth >= 7) {
        days = 31;
        if (yr <= 2020) {
        frame = -84;//Wed
        }
        else if (yr <= 2021) {
        frame = -56;//Thu
        }
        else if (yr <= 2022) {
        frame = -28;//Fri
        }
        else if (yr <= 2023) {
        frame = 1;//Sat
        }
        else if (yr <= 2024) {
        frame = -140;//Mon
        }
        else if (yr <= 2025) {
        frame = -112;//Tue
        }
        else if (yr <= 2026) {
        frame = -84;//Wed
        }
        else if (yr <= 2027) {
        frame = -56;//Thu
        }
        else if (yr <= 2028) {
        frame = 1;//Sat
        }
        else if (yr <= 2029) {
        frame = -168;//Sun
        }
        else if (yr <= 2030) {
        frame = -140;//Mon;
        }
        else if (yr <= 2031) {
        frame = -112;//Tue
        }
        else if (yr <= 2032) {
        frame = -56;//Thu
        }
        else if (yr <= 2033) {
        frame = -28;//Fri
        }
        else if (yr <= 2034) {
        frame = 1;//Sat
        }
        else if (yr <= 2035) {
        frame = -168;//Sun
        }
        else if (yr <= 2036) {
        frame = -112;//Tue
        }
        else if (yr <= 2037) {
        frame = -84;//Wed
        }
        else if (yr <= 2038) {
        frame = -56;//Thu
        }
        else if (yr <= 2039) {
        frame = -28;//Fri
        }
        else if (yr <= 2040) {
        frame = -168;//Sun
        }
        }
        else if (mth >= 6) {
        days = 30;
        if (yr <= 2020) {
        frame = -140;//Mon
        }
        else if (yr <= 2021) {
        frame = -112;//Tue
        }
        else if (yr <= 2022) {
        frame = -84;//Wed
        }
        else if (yr <= 2023) {
        frame = -56;//Thu
        }
        else if (yr <= 2024) {
        frame = 1;//Sat
        }
        else if (yr <= 2025) {
        frame = -168;//Sun
        }
        else if (yr <= 2026) {
        frame = -140;//Mon
        }
        else if (yr <= 2027) {
        frame = -112;//Tue
        }
        else if (yr <= 2028) {
        frame = -56;//Thu
        }
        else if (yr <= 2029) {
        frame = -28;//Fri
        }
        else if (yr <= 2030) {
        frame = 1;//Sat
        }
        else if (yr <= 2031) {
        frame = -168;//Sun
        }
        else if (yr <= 2032) {
        frame = -112;//Tue
        }
        else if (yr <= 2033) {
        frame = -84;//Wed
        }
        else if (yr <= 2034) {
        frame = -56;//Thu
        }
        else if (yr <= 2035) {
        frame = -28;//Fri
        }
        else if (yr <= 2036) {
        frame = -168;//Sun
        }
        else if (yr <= 2037) {
        frame = -140;//Mon;
        }
        else if (yr <= 2038) {
        frame = -112;//Tue
        }
        else if (yr <= 2039) {
        frame = -84;//Wed
        }
        else if (yr <= 2040) {
        frame = -28;//Fri
        }
        }
        else if (mth >= 5) {
        days = 31;
        if (yr <= 2020) {
        frame = -28;//Fri
        }
        else if (yr <= 2021) {
        frame = 1;//Sat
        }
        else if (yr <= 2022) {
        frame = -168;//Sun
        }
        else if (yr <= 2023) {
        frame = -140;//Mon
        }
        else if (yr <= 2024) {
        frame = -84;//Wed
        }
        else if (yr <= 2025) {
        frame = -56;//Thu
        }
        else if (yr <= 2026) {
        frame = -28;//Fri
        }
        else if (yr <= 2027) {
        frame = 1;//Sat
        }
        else if (yr <= 2028) {
        frame = -140;//Mon
        }
        else if (yr <= 2029) {
        frame = -112;//Tue
        }
        else if (yr <= 2030) {
        frame = -84;//Wed
        }
        else if (yr <= 2031) {
        frame = -56;//Thu
        }
        else if (yr <= 2032) {
        frame = 1;//Sat
        }
        else if (yr <= 2033) {
        frame = -168;//Sun
        }
        else if (yr <= 2034) {
        frame = -140;//Mon
        }
        else if (yr <= 2035) {
        frame = -112;//Tue
        }
        else if (yr <= 2036) {
        frame = -56;//Thu
        }
        else if (yr <= 2037) {
        frame = -28;//Fri
        }
        else if (yr <= 2038) {
        frame = 1;//Sat
        }
        else if (yr <= 2039) {
        frame = -168;//Sun
        }
        else if (yr <= 2040) {
        frame = -112;//Tue
        }
        }
        else if (mth >= 4) {
        days = 30;
        if (yr <= 2020) {
        frame = -84;//Wed
        }
        else if (yr <= 2021) {
        frame = -56;//Thu
        }
        else if (yr <= 2022) {
        frame = -28;//Fri
        }
        else if (yr <= 2023) {
        frame = 1;//Sat
        }
        else if (yr <= 2024) {
        frame = -140;//Mon
        }
        else if (yr <= 2025) {
        frame = -112;//Tue
        }
        else if (yr <= 2026) {
        frame = -84;//Wed
        }
        else if (yr <= 2027) {
        frame = -56;//Thu
        }
        else if (yr <= 2028) {
        frame = 1;//Sat
        }
        else if (yr <= 2029) {
        frame = -168;//Sun
        }
        else if (yr <= 2030) {
        frame = -140;//Mon
        }
        else if (yr <= 2031) {
        frame = -112;//Tue
        }
        else if (yr <= 2032) {
        frame = -56;//Thu
        }
        else if (yr <= 2033) {
        frame = -28;//Fri
        }
        else if (yr <= 2034) {
        frame = 1;//Sat
        }
        else if (yr <= 2035) {
        frame = -168;//Sun
        }
        else if (yr <= 2036) {
        frame = -112;//Tue
        }
        else if (yr <= 2037) {
        frame = -84;//Wed
        }
        else if (yr <= 2038) {
        frame = -56;//Thu
        }
        else if (yr <= 2039) {
        frame = -28;//Fri
        }
        else if (yr <= 2040) {
        frame = -168;//Sun
        }
        }
        else if (mth >= 3) {
        days = 31;
        if (yr <= 2020) {
        frame = -168;//Sun
        }
        else if (yr <= 2021) {
        frame = -140;//Mon
        }
        else if (yr <= 2022) {
        frame = -112;//Tue
        }
        else if (yr <= 2023) {
        frame = -84;//Wed
        }
        else if (yr <= 2024) {
        frame = -28;//Fri
        }
        else if (yr <= 2025) {
        frame = 1;//Sat
        }
        else if (yr <= 2026) {
        frame = -168;//Sun
        }
        else if (yr <= 2027) {
        frame = -140;//Mon
        }
        else if (yr <= 2028) {
        frame = -84;//Wed
        }
        else if (yr <= 2029) {
        frame = -56;//Thu
        }
        else if (yr <= 2030) {
        frame = -28;//Fri
        }
        else if (yr <= 2031) {
        frame = 1;//Sat
        }
        else if (yr <= 2032) {
        frame = -140;//Mon
        }
        else if (yr <= 2033) {
        frame = -112;//Tue
        }
        else if (yr <= 2034) {
        frame = -84;//Wed
        }
        else if (yr <= 2035) {
        frame = -56;//Thu
        }
        else if (yr <= 2036) {
        frame = 1;//Sat
        }
        else if (yr <= 2037) {
        frame = -168;//Sun
        }
        else if (yr <= 2038) {
        frame = -140;//Mon
        }
        else if (yr <= 2039) {
        frame = -112;//Tue
        }
        else if (yr <= 2040) {
        frame = -56;//Thu
        }
        }
        else if (mth >= 2) {
        if (yr <= 2020) {
        days = 29;
        frame = 1;//Sat
        }
        else if (yr <= 2021) {
        days = 28;
        frame = -140;//Mon
        }
        else if (yr <= 2022) {
        days = 28;
        frame = -112;//Tue
        }
        else if (yr <= 2023) {
        days = 28;
        frame = -84;//Wed
        }
        else if (yr <= 2024) {
        days = 29;
        frame = -56;//Thu
        }
        else if (yr <= 2025) {
        days = 28;
        frame = 1;//Sat
        }
        else if (yr <= 2026) {
        days = 28;
        frame = -168;//Sun
        }
        else if (yr <= 2027) {
        days = 28;
        frame = -140;//Mon
        }
        else if (yr <= 2028) {
        days = 29;
        frame = -112;//Tue
        }
        else if (yr <= 2029) {
        days = 28;
        frame = -56;//Thu
        }
        else if (yr <= 2030) {
        days = 28;
        frame = -28;//Fri
        }
        else if (yr <= 2031) {
        days = 28;
        frame = 1;//Sat
        }
        else if (yr <= 2032) {
        days = 29;
        frame = -168;//Sun
        }
        else if (yr <= 2033) {
        days = 28;
        frame = -112;//Tue
        }
        else if (yr <= 2034) {
        days = 28;
        frame = -84;//Wed
        }
        else if (yr <= 2035) {
        days = 28;
        frame = -56;//Thu
        }
        else if (yr <= 2036) {
        days = 29;
        frame = -28;//Fri
        }
        else if (yr <= 2037) {
        days = 28;
        frame = -168;//Sun
        }
        else if (yr <= 2038) {
        days = 28;
        frame = -140;//Mon
        }
        else if (yr <= 2039) {
        days = 28;
        frame = -112;//Tue
        }
        else if (yr <= 2040) {
        days = 29;
        frame = -84;//Wed
        }
        }
        else if (mth >= 1) {
        days = 31;
        if (yr <= 2020) {
        frame =  -84;//Wed
        }
        else if (yr <= 2021) {
        frame = -28;//Fri
        }
        else if (yr <= 2022) {
        frame = 1;//Sat
        }
        else if (yr <= 2023) {
        frame = -168;//Sun
        }
        else if (yr <= 2024) {
        frame = -140;//Mon
        }
        else if (yr <= 2025) {
        frame = -84;//Wed
        }
        else if (yr <= 2026) {
        frame = -56;//Thu
        }
        else if (yr <= 2027) {
        frame = -28;//Fri
        }
        else if (yr <= 2028) {
        frame = 1;//Sat
        }
        else if (yr <= 2029) {
        frame = -140;//Mon
        }
        else if (yr <= 2030) {
        frame = -112;//Tue
        }
        else if (yr <= 2031) {
        frame = -84;//Wed
        }
        else if (yr <= 2032) {
        frame = -56;//Thu
        }
        else if (yr <= 2033) {
        frame = 1;//Sat
        }
        else if (yr <= 2034) {
        frame = -168;//Sun
        }
        else if (yr <= 2035) {
        frame = -140;//Mon
        }
        else if (yr <= 2036) {
        frame = -112;//Tue
        }
        else if (yr <= 2037) {
        frame = -56;//Thu
        }
        else if (yr <= 2038) {
        frame = -28;//Fri
        }
        else if (yr <= 2039) {
        frame = 1;//Sat
        }
        else if (yr <= 2040) {
        frame = -168;//Sun
        }
        }

//      Ref: frame= -168:1=Sun, -140:1=Mon, -112:1=Tue, -84::1=Wed, -56:1=Thu, -28:1=Fri, 1:1=Sat
        if (days >= 31) {
        UiSDK::drawBitmap(display, frame, 58, calendar31n, 367, 142, fg);
        }
        else if (days >= 30) {
        UiSDK::drawBitmap(display, frame, 58, calendar30n, 367, 142, fg);
        }
        else if (days >= 29) {
        UiSDK::drawBitmap(display, frame, 58, calendar29n, 367, 142, fg);
        }
        else if (days >= 28) {
        UiSDK::drawBitmap(display, frame, 58, calendar28n, 367, 142, fg);
        }


    UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, 100-w/2, 38); // date position
    UiSDK::print(display, textstring);
    // END draw Calendar
      
    // highlight calendar day
        textstring = dayShortStr(currentTime.Wday);
        if (textstring == "Sun") {
        wkday = 1;
        }
        else if (textstring == "Mon") {
        wkday = 2;
        }
        else if (textstring == "Tue") {
        wkday = 3;
        }
        else if (textstring == "Wed") {
        wkday = 4;
        }
        else if (textstring == "Thu") {
        wkday = 5;
        }
        else if (textstring == "Fri") {
        wkday = 6;
        }
        else if (textstring == "Sat") {
        wkday = 7;
        }

        days = currentTime.Day;
        y = 56;//R1
        if (wkday <= 1) {
        x = -1;//Sun
        if (days <= 1) {
        y += 0;//R1
        }
        else if (days <= 8) {
        y += 28;//R2
        }
        else if (days <= 15) {
        y += 56;//R3
        }
        else if (days <= 22) {
        y += 84;//R4
        }
        else if (days <= 31) {
        y += 112;//R5
        }
        }
        else if (wkday <= 2) {
        x = 27;//Mon
        if (days <= 2) {
        y += 0;
        }
        else if (days <= 9) {
        y += 28;//R2
        }
        else if (days <= 16) {
        y += 56;//R3
        }
        else if (days <= 23) {
        y += 84;//R4
        }
        else if (days <= 31) {
        y += 112;//R5
        }
        }
        else if (wkday <= 3) {
        x = 55;//Tue
        if (days <= 3) {
        y += 0;
        }
        else if (days <= 10) {
        y += 28;//R2
        }
        else if (days <= 17) {
        y += 56;//R3
        }
        else if (days <= 24) {
        y += 84;//R4
        }
        else if (days <= 31) {
        y += 112;//R5
        }
        }
        else if (wkday <= 4) {
        x = 83;//Wed
        if (days <= 4) {
        y += 0;
        }
        else if (days <= 11) {
        y += 28;//R2
        }
        else if (days <= 18) {
        y += 56;//R3
        }
        else if (days <= 25) {
        y += 84;//R4
        }
        else if (days <= 31) {
        y += 112;//R5
        }
        }
        else if (wkday <= 5) {
        x = 112;//Thu
        y += 0;//R2
        if (days <= 5) {
        y += 0;
        }
        else if (days <= 12) {
        y += 28;//R2
        }
        else if (days <= 19) {
        y += 56;//R3
        }
        else if (days <= 26) {
        y += 84;//R4
        }
        else if (days <= 31) {
        y += 112;//R5
        }
        }
        else if (wkday <= 6) {
        x = 140;//Fri
        if (days <= 6) {
        y += 0;
        }
        else if (days <= 13) {
        y += 28;//R2
        }
        else if (days <= 20) {
        y += 56;//R3
        }
        else if (days <= 27) {
        y += 84;//R4
        }
        else if (days <= 31) {
        y += 112;//R5
        }
        }
        else if (wkday <= 7) {
        x = 168;//Sat
        if (days <= 7) {
        y += 0;
        }
        else if (days <= 14) {
        y += 28;//R2
        }
        else if (days <= 21) {
        y += 56;//R3
        }
        else if (days <= 28) {
        y += 84;//R4
        }
        else if (days <= 31) {
        y += 112;//R5
        }
        }
        
        //draw day hightlight image
        if (wkday == 4) {
        UiSDK::drawBitmap(display, x,y, block35x34, 35, 34, fg);
        }
        UiSDK::drawBitmap(display, x,y, block34x34, 35, 34, fg);
        //END highlight calendar day
  
/*      //draw day hightlight image
      x = 27;//Sun:x=-1, Mon:x=27, Tue:x=55, Thu:x=112, Fri:x=140, Sat:x=168
      y = 140;//R1:y=56, R2:y=84, R3:y=112, R4:y=140, R5:y=168
  UiSDK::drawBitmap(display, x,y, block34x34, 35, 34, fg);
      x = 83;//Wed:x=83
      y = 140;//R1:y=56, R2:y=84, R3:y=112, R4:y=140, R5:y=168
  UiSDK::drawBitmap(display, x,y, block35x34, 35, 34, fg);
      //END highlight calendar day
*/      
    // draw time
    UiSDK::setFont(display, &Teko_Regular12pt7b);
    int displayHour;
    if (currentTime.Hour > 12) {
      textstring = ((currentTime.Hour+11)%12)+1;
    } else if (currentTime.Hour < 1) {
      textstring = 12;
    } else {
      textstring = currentTime.Hour;
    }
    textstring += ":";
    if (currentTime.Minute < 10) {
      textstring += "0";
    } else {
      textstring += "";
    }
    textstring += currentTime.Minute;
    //This option displays am or pm to 12 hour clock
    if (currentTime.Hour >= 12) {
      textstring += "PM";
    } else {
      textstring += "AM";
    }
    
    UiSDK::setCursor(display, 5, 19);
    UiSDK::print(display, textstring);
    //END time

    //  Draw Battery Percent
     UiSDK::setFont(display, &Teko_Regular12pt7b);
     int8_t batteryLevel = 0;
     float VBAT = getBatteryVoltage();

     if(VBAT >= 4.2){
        batteryLevel = 100.0;
     }
     else if (VBAT >= 3.3) {
        batteryLevel = 100.0*(VBAT-3.3)/0.9;
    }
      textstring = "BAT:";
      textstring += batteryLevel;
      textstring += "%";
      UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
      UiSDK::setCursor(display, 195-w, 19);
      UiSDK::print(display, textstring);
     //  END OF Draw Battery

    }

};

void WatchfaceEntrypoint_CalendarWatchFace(Watchy &watchy) {
  CalendarWatchFace face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}

void showWatchFace_Calendar_WatchFace(Watchy &watchy) {
  WatchfaceEntrypoint_CalendarWatchFace(watchy);
}

#endif // WATCHY_WRAPPER_INCLUDE || WATCHY_STANDALONE_WATCHFACE
