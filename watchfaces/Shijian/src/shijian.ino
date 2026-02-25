#include "../../../src/watchy/Watchy.h"  //include the Watchy library
#include "../../../src/settings/settings.h"
#include "SuXinPoemSeal24pt7b.h"
#include "HanDan24pt7b.h"

#define RECT_TOP_PX 40
#define RECT_LEFT_PX 0
#define RECT_RADIUS 5


class Shijian : public Watchy {  //inherit and extend Watchy class
  using Watchy::Watchy;
public:
	
	void drawWatchFace() { //override this method to customize how the watch face looks
		uint16_t lines = 1;

		const String lclows[10] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J" };
		const String lcteensone[11] = { "", "K", "K", "K", "K", "K", "K", "K", "K", "K", "K" };
		const String lcteenstwo[11] = { "", "B", "C", "D", "E", "F", "G", "H", "I", "J", "" };
		const String lctens[10] = { "A", "K", "CK", "DK", "EK", "FK", "GK", "HK", "IK", "JK" };

		const String uclows[10] = { "A", "L", "M", "N", "O", "P", "Q", "R", "S", "T" };
		const String ucteensone[11] = { "", "U", "U", "U", "U", "U", "U", "U", "U", "U", "U" };
		const String ucteenstwo[11] = { "", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "" };
		const String uctens[10] = { "A", "U", "MU", "NU", "OU", "PU", "QU", "RU", "SU", "TU" };

		//drawbg
		display.fillScreen(GxEPD_WHITE);
		display.setTextColor(GxEPD_WHITE);
		display.fillRoundRect(RECT_LEFT_PX, RECT_TOP_PX, 200 - 2 * RECT_LEFT_PX, 200 - 2 * RECT_TOP_PX, RECT_RADIUS, GxEPD_BLACK);

		String hourStr, minuteStr;
		int16_t  x1, y1;
		uint16_t w, h;

		//drawtime
		lines += 1;
		display.setFont(&SuXinPoemSeal24pt7b);
		if (currentTime.Hour == 0) {
			hourStr = uctens[2] + uclows[4];
		}
		else if (currentTime.Hour < 10) {
			hourStr = uclows[currentTime.Hour];
		}
		else if (currentTime.Hour < 20) {
			hourStr = ucteensone[currentTime.Hour - 9];
			if (currentTime.Hour > 10) {
				hourStr += ucteenstwo[currentTime.Hour % 10];
			}
		}
		else {
			hourStr = uctens[currentTime.Hour / 10];
			if (currentTime.Hour % 10 > 0) {
				hourStr += uclows[currentTime.Hour % 10];
			}
		}
		hourStr += "V";
		display.getTextBounds(hourStr, 0, 0, &x1, &y1, &w, &h);
		display.setCursor(100 - w / 2, lines * 47 - 2);
		display.print(hourStr);

		lines += 1;
		display.setCursor(5, lines * 47 - 2);
		display.setFont(&HanDan24pt7b);
		if (currentTime.Minute == 0) {
			minuteStr = "X";
		}
		else {
			if (currentTime.Minute < 10) {
				minuteStr = "A" + lclows[currentTime.Minute];
			}
			else if (currentTime.Minute < 20) {
				minuteStr = lcteensone[currentTime.Minute - 9];
				if (currentTime.Minute > 10) {
					minuteStr += lcteenstwo[currentTime.Minute % 10];
				}
			}
			else {
				minuteStr = lctens[currentTime.Minute / 10];
				if (currentTime.Minute % 10 > 0) {
					minuteStr += lclows[currentTime.Minute % 10];
				}
			}
			minuteStr += "W";
		}
		display.getTextBounds(minuteStr, 0, 0, &x1, &y1, &w, &h);
		display.setCursor(100 - w / 2 - 5, lines * 47 - 2 + 5);
		display.print(minuteStr);
	}
};

Shijian m(settings);  //instantiate your watchface

void setup() {
  m.init();  //call init in setup
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}