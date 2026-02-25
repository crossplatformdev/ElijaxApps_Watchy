#if defined(WATCHY_WRAPPER_INCLUDE) || defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../../watchy/Watchy.h"  //include the Watchy library
#include "../../../sdk/UiSDK.h"
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

		UiSDK::initScreen(display);
		const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
		const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

		const String lclows[10] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J" };
		const String lcteensone[11] = { "", "K", "K", "K", "K", "K", "K", "K", "K", "K", "K" };
		const String lcteenstwo[11] = { "", "B", "C", "D", "E", "F", "G", "H", "I", "J", "" };
		const String lctens[10] = { "A", "K", "CK", "DK", "EK", "FK", "GK", "HK", "IK", "JK" };

		const String uclows[10] = { "A", "L", "M", "N", "O", "P", "Q", "R", "S", "T" };
		const String ucteensone[11] = { "", "U", "U", "U", "U", "U", "U", "U", "U", "U", "U" };
		const String ucteenstwo[11] = { "", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "" };
		const String uctens[10] = { "A", "U", "MU", "NU", "OU", "PU", "QU", "RU", "SU", "TU" };

		//drawbg
		UiSDK::setTextColor(display, bgColor);
		UiSDK::fillRoundRect(display, RECT_LEFT_PX, RECT_TOP_PX, 200 - 2 * RECT_LEFT_PX, 200 - 2 * RECT_TOP_PX, RECT_RADIUS, fgColor);

		String hourStr, minuteStr;
		int16_t  x1, y1;
		uint16_t w, h;

		//drawtime
		lines += 1;
		UiSDK::setFont(display, &SuXinPoemSeal24pt7b);
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
		UiSDK::getTextBounds(display, hourStr, 0, 0, &x1, &y1, &w, &h);
		UiSDK::setCursor(display, 100 - w / 2, lines * 47 - 2);
		UiSDK::print(display, hourStr);

		lines += 1;
		UiSDK::setCursor(display, 5, lines * 47 - 2);
		UiSDK::setFont(display, &HanDan24pt7b);
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
		UiSDK::getTextBounds(display, minuteStr, 0, 0, &x1, &y1, &w, &h);
		UiSDK::setCursor(display, 100 - w / 2 - 5, lines * 47 - 2 + 5);
		UiSDK::print(display, minuteStr);
	}
};

void WatchfaceEntrypoint_Shijian(Watchy &watchy) {
	Shijian face;
	face.settings = watchy.settings;
	face.currentTime = watchy.currentTime;
	face.drawWatchFace();
}

void showWatchFace_Shijian(Watchy &watchy) {
	WatchfaceEntrypoint_Shijian(watchy);
}

#ifdef WATCHY_STANDALONE_WATCHFACE
Shijian m;  //instantiate your watchface

void setup() {
	m.init();  //call init in setup
}

void loop() {
	// this should never run, Watchy deep sleeps after init();
}
#endif

#endif // WATCHY_WRAPPER_INCLUDE || WATCHY_STANDALONE_WATCHFACE