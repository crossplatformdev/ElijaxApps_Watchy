#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"
#include "../sdk/NetUtils.h"

namespace {

static const uint8_t MAX_STATIONS     = 8;
static const uint8_t VISIBLE_STATIONS = 6;

struct StationList {
	String names[MAX_STATIONS];
	String urls[MAX_STATIONS];
	uint8_t count = 0;
};

// TuneIn OPML presets.
static String kTuneInPresetsUrl = String("https://opml.radiotime.com/Browse.ashx?c=presets");

static bool fetchTuneInStations(Watchy &watchy, StationList &out, String &err) {
	out.count = 0;
	err = "";

	if (!NetUtils::ensureWiFi(watchy, 1, 6000)) {
		err = "WiFi not connected";
		return false;
	}

	NetResponse resp = NetUtils::httpGet(kTuneInPresetsUrl, 8000, false);
	if (resp.httpCode != 200 || resp.body.length() == 0) {
		err = resp.error.length() ? resp.error : String("HTTP ") + resp.httpCode;
		return false;
	}

	String payload = resp.body;
	int pos = 0;
	while (out.count < MAX_STATIONS) {
		int o = payload.indexOf("<outline", pos);
		if (o < 0) {
			break;
		}
		int textPos = payload.indexOf("text=\"", o);
		if (textPos < 0) {
			pos = o + 8;
			continue;
		}
		textPos += 6;
		int textEnd = payload.indexOf('"', textPos);
		if (textEnd < 0) {
			break;
		}
		String name = payload.substring(textPos, textEnd);

		int urlPos = payload.indexOf("URL=\"", o);
		if (urlPos < 0) {
			pos = textEnd + 1;
			continue;
		}
		urlPos += 5;
		int urlEnd = payload.indexOf('"', urlPos);
		if (urlEnd < 0) {
			break;
		}
		String url = payload.substring(urlPos, urlEnd);

		if (name.length() == 0 || url.length() == 0) {
			pos = urlEnd + 1;
			continue;
		}

		if (name.length() > 40) {
			name = name.substring(0, 37) + "...";
		}

		out.names[out.count] = name;
		out.urls[out.count]  = url;
		++out.count;

		pos = urlEnd + 1;
	}

	if (out.count == 0) {
		err = "No stations parsed";
		return false;
	}

	return true;
}

enum class RadioScreen : uint8_t { Prompt, Fetching, Error, List, Detail };

static StationList sStations;
static String sFetchError;
static RadioScreen sScreen = RadioScreen::Prompt;
static int8_t sSelectedIndex = -1;
static uint8_t sStartIndex = 0;
static int8_t sDetailIndex = -1;
static volatile bool sExit = false;
static volatile bool sDirty = true;

static void radioBack(Watchy *watchy) {
	UiTemplates::waitForAllButtonsReleased(50, 100);
	if (sScreen == RadioScreen::Detail) {
		sScreen = RadioScreen::List;
		sDirty = true;
		return;
	}
	sExit = true;
}

static void radioMenu(Watchy *watchy) {
	UiTemplates::waitForAllButtonsReleased(50, 100);
	if (sScreen == RadioScreen::Prompt || sScreen == RadioScreen::Error) {
		sScreen = RadioScreen::Fetching;
		sDirty = true;
		return;
	}
	if (sScreen == RadioScreen::List) {
		if (sStations.count > 0 && sSelectedIndex >= 0 && sSelectedIndex < static_cast<int8_t>(sStations.count)) {
			sDetailIndex = sSelectedIndex;
			sScreen = RadioScreen::Detail;
			sDirty = true;
		}
		return;
	}
}

static void radioUp(Watchy *watchy) {
	UiTemplates::waitForAllButtonsReleased(50, 100);
	if (sScreen != RadioScreen::List) return;
	if (sStations.count == 0) return;
	if (sSelectedIndex < 0) {
		sSelectedIndex = 0;
		sDirty = true;
		return;
	}
	if (sSelectedIndex > 0) {
		--sSelectedIndex;
	} else {
		sSelectedIndex = static_cast<int8_t>(sStations.count - 1);
	}
	sDirty = true;
}

static void radioDown(Watchy *watchy) {
	UiTemplates::waitForAllButtonsReleased(50, 100);
	if (sScreen != RadioScreen::List) return;
	if (sStations.count == 0) return;
	if (sSelectedIndex < 0) {
		sSelectedIndex = 0;
		sDirty = true;
		return;
	}
	if (sSelectedIndex < static_cast<int8_t>(sStations.count - 1)) {
		++sSelectedIndex;
	} else {
		sSelectedIndex = 0;
	}
	sDirty = true;
}

} // namespace

void Watchy::showRadio() {
	guiState = APP_STATE;

	setButtonHandlers(radioBack, radioUp, radioMenu, radioDown);

	sStations.count = 0;
	sFetchError = "";
	sScreen = RadioScreen::Prompt;
	sSelectedIndex = -1;
	sStartIndex = 0;
	sDetailIndex = -1;
	sExit = false;
	sDirty = true;

	while (!sExit) {
		UIControlsRowLayout controls[4] = {
			{"BACK", &Watchy::backPressed},
			{"-", nullptr},
			{"-", nullptr},
			{"-", nullptr},
		};

		if (sScreen == RadioScreen::Prompt) {
			controls[2] = {"FETCH", &Watchy::menuPressed};
		} else if (sScreen == RadioScreen::Error) {
			controls[2] = {"RETRY", &Watchy::menuPressed};
		} else if (sScreen == RadioScreen::List) {
			controls[1] = {"UP", &Watchy::upPressed};
			controls[2] = {"DETAIL", &Watchy::menuPressed};
			controls[3] = {"DOWN", &Watchy::downPressed};
		} else if (sScreen == RadioScreen::Fetching) {
			controls[0] = {"-", nullptr};
		}

		// Pump input frequently.
		UiSDK::renderControlsRow(*this, controls);
		if (!sDirty) {
			delay(10);
			continue;
		}
		sDirty = false;

		if (sScreen == RadioScreen::Prompt) {
			UITextSpec header{};
			header.x    = 0;
			header.y    = 36;
			header.font = &FreeMonoBold9pt7b;
			header.text = "TuneIn presets";

			UIScrollableTextSpec body{};
			body.x              = 0;
			body.y              = 56;
			body.w              = WatchyDisplay::WIDTH;
			body.h              = WatchyDisplay::HEIGHT - 80;
			body.font           = &FreeMonoBold9pt7b;
			body.text           = "MENU: fetch list\nBACK: cancel\n\nStreaming not available.";
			body.firstLine      = 0;
			body.maxLines       = 6;
			body.lineHeight     = 14;

			UIAppSpec app{};
			app.texts           = &header;
			app.textCount       = 1;
			app.scrollTexts     = &body;
			app.scrollTextCount = 1;
			app.controls[0] = controls[0];
			app.controls[1] = controls[1];
			app.controls[2] = controls[2];
			app.controls[3] = controls[3];
			UiSDK::renderApp(*this, app);
			continue;
		}

		if (sScreen == RadioScreen::Fetching) {
			UITextSpec header{};
			header.x    = 0;
			header.y    = 36;
			header.font = &FreeMonoBold9pt7b;
			header.text = "Fetching TuneIn...";

			UIAppSpec app{};
			app.texts     = &header;
			app.textCount = 1;
			app.controls[0] = controls[0];
			app.controls[1] = controls[1];
			app.controls[2] = controls[2];
			app.controls[3] = controls[3];
			UiSDK::renderApp(*this, app);

			bool ok = fetchTuneInStations(*this, sStations, sFetchError);
			if (!ok) {
				sScreen = RadioScreen::Error;
				sDirty = true;
				continue;
			}

			sSelectedIndex = (sStations.count > 0) ? 0 : -1;
			sStartIndex = 0;
			sScreen = RadioScreen::List;
			sDirty = true;
			continue;
		}

		if (sScreen == RadioScreen::Error) {
			UITextSpec header{};
			header.x    = 0;
			header.y    = 36;
			header.font = &FreeMonoBold9pt7b;
			header.text = "TuneIn error";

			UIScrollableTextSpec body{};
			body.x          = 0;
			body.y          = 56;
			body.w          = WatchyDisplay::WIDTH;
			body.h          = WatchyDisplay::HEIGHT - 80;
			body.font       = &FreeMonoBold9pt7b;
			body.text       = String("Could not load presets.\n\n") + sFetchError +
												"\n\nMENU: retry\nBACK: cancel";
			body.firstLine  = 0;
			body.maxLines   = 8;
			body.lineHeight = 14;

			UIAppSpec app{};
			app.texts           = &header;
			app.textCount       = 1;
			app.scrollTexts     = &body;
			app.scrollTextCount = 1;
			app.controls[0] = controls[0];
			app.controls[1] = controls[1];
			app.controls[2] = controls[2];
			app.controls[3] = controls[3];
			UiSDK::renderApp(*this, app);
			continue;
		}

		if (sScreen == RadioScreen::Detail && sDetailIndex >= 0 && sDetailIndex < sStations.count) {
			UITextSpec header{};
			header.x    = 0;
			header.y    = 36;
			header.font = &FreeMonoBold9pt7b;
			header.text = "Station info";

			UIScrollableTextSpec body{};
			body.x          = 0;
			body.y          = 56;
			body.w          = WatchyDisplay::WIDTH;
			body.h          = WatchyDisplay::HEIGHT - 80;
			body.font       = &FreeMonoBold9pt7b;
			body.text       = String("Name: ") + sStations.names[sDetailIndex] +
											"\nURL: \n" + sStations.urls[sDetailIndex] +
												"\n\nNote: audio playback not available.\nBACK: list";
			body.firstLine  = 0;
			body.maxLines   = 10;
			body.lineHeight = 14;

			UIAppSpec app{};
			app.texts           = &header;
			app.textCount       = 1;
			app.scrollTexts     = &body;
			app.scrollTextCount = 1;
			app.controls[0] = controls[0];
			app.controls[1] = controls[1];
			app.controls[2] = controls[2];
			app.controls[3] = controls[3];
			UiSDK::renderApp(*this, app);
			continue;
		}

		// Screen::List
		UITextSpec header{};
		header.x    = 0;
		header.y    = 36;
		header.font = &FreeMonoBold9pt7b;
		header.text = "TuneIn presets";

		String info;
		if (sStations.count == 0) {
			info = "No stations parsed";
		} else {
			info = "UP/DOWN: select\nMENU: details";
		}

		UIScrollableTextSpec scroll{};
		scroll.x              = 0;
		scroll.y              = 56;
		scroll.w              = WatchyDisplay::WIDTH;
		scroll.h              = 28;
		scroll.font           = &FreeMonoBold9pt7b;
		scroll.fillBackground = false;
		scroll.text           = info;
		scroll.firstLine      = 0;
		scroll.maxLines       = 2;
		scroll.lineHeight     = 14;

		static UIMenuItemSpec items[MAX_STATIONS];
		for (uint8_t i = 0; i < sStations.count && i < MAX_STATIONS; ++i) {
			items[i].label = sStations.names[i].c_str();
		}

		UIMenuSpec menu{};
		menu.x             = 0;
		menu.y             = 96;
		menu.itemHeight    = MENU_HEIGHT;
		menu.font          = &FreeMonoBold9pt7b;
		menu.items         = items;
		menu.itemCount     = sStations.count;
		menu.selectedIndex = sSelectedIndex;

		uint8_t visibleRows = VISIBLE_STATIONS;
		if (visibleRows > sStations.count) {
			visibleRows = sStations.count;
		}
		if (sStations.count > 0) {
			// Clamp selection to available stations.
			if (sSelectedIndex < 0) {
				sSelectedIndex = 0;
			} else if (sSelectedIndex >= static_cast<int8_t>(sStations.count)) {
				sSelectedIndex = static_cast<int8_t>(sStations.count - 1);
			}
			UiTemplates::keepMenuSelectionVisible(static_cast<uint8_t>(sSelectedIndex),
										 visibleRows,
									 sStations.count,
									 sStartIndex);
		} else {
			sStartIndex = 0;
		}
		menu.startIndex   = sStartIndex;
		menu.visibleCount = visibleRows;

		UIAppSpec app{};
		app.texts           = &header;
		app.textCount       = 1;
		app.scrollTexts     = &scroll;
		app.scrollTextCount = 1;
		app.menus           = (sStations.count > 0) ? &menu : nullptr;
		app.menuCount       = (sStations.count > 0) ? 1 : 0;
		app.controls[0] = controls[0];
		app.controls[1] = controls[1];
		app.controls[2] = controls[2];
		app.controls[3] = controls[3];
		UiSDK::renderApp(*this, app);
	}

	clearButtonHandlers();
	delay(100);
	showMenu(menuIndex);
}

