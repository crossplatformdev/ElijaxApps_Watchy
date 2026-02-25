#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/Fonts.h"
#include "../sdk/NetUtils.h"
#include "../sdk/UiTemplates.h"

namespace {

static bool isUnreservedFormChar(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
		   c == '-' || c == '_' || c == '.' || c == '~';
}

static String urlEncodeFormComponent(const String &s) {
	String out;
	out.reserve(s.length() * 2);
	for (uint32_t i = 0; i < s.length(); ++i) {
		const char c = s.charAt(i);
		if (c == ' ') {
			out += '+';
			continue;
		}
		if (isUnreservedFormChar(c)) {
			out += c;
			continue;
		}
		char buf[4];
		snprintf(buf, sizeof(buf), "%%%02X", static_cast<unsigned char>(c));
		out += buf;
	}
	return out;
}

} // namespace

void Watchy::showGoogleSearch() {
	guiState = APP_STATE;

	char query[256] = {0};
	if (!NetUtils::editTarget(*this, query, sizeof(query), "Search query")) {
		showMenu(menuIndex);
		return;
	}

	// Quick feedback before entering the browser.

	UITextSpec header{};
	header.x = 0;
	header.y = 36;
	header.font = &FreeMonoBold9pt7b;
	header.text = "DuckDuckGo";

	UIScrollableTextSpec body{};
	body.x = 0;
	body.y = 54;
	body.w = WatchyDisplay::WIDTH;
	body.h = WatchyDisplay::HEIGHT - 80;
	body.font = UiSDK::tinyMono6x8();
	body.fillBackground = false;
	body.centered = false;
	body.text = String("Searching for:\n") + query;
	body.firstLine = 0;
	body.maxLines = 8;
	body.lineHeight = 14;

	UIAppSpec app{};
	app.texts = &header;
	app.textCount = 1;
	app.scrollTexts = &body;
	app.scrollTextCount = 1;
	UiTemplates::renderBarePage(*this, app);

	if (!NetUtils::ensureWiFi(*this, 1, 8000)) {
		return;
	}

	const String url = "https://html.duckduckgo.com/html/";
	const String form = String("q=") + urlEncodeFormComponent(String(query)) + "&b=";

	showTextBrowserPostForm(url,
						form,
						"https://html.duckduckgo.com",
						"https://html.duckduckgo.com/",
						"es-ES,es;q=0.9,en;q=0.8");
}

#if 0

static String stripTags(const String &in) {
	String out;
	out.reserve(in.length());
	bool inTag = false;
	for (uint32_t i = 0; i < in.length(); ++i) {
		const char c = in.charAt(i);
		if (c == '<') {
			inTag = true;
			continue;
		}
		if (c == '>') {
			inTag = false;
			continue;
		}
		if (!inTag) {
			out += c;
		}
	}
	out.trim();
	return out;
}

static bool isUnreservedFormChar(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
		   c == '-' || c == '_' || c == '.' || c == '~';
}

static String urlEncodeFormComponent(const String &s) {
	String out;
	out.reserve(s.length() * 2);
	for (uint32_t i = 0; i < s.length(); ++i) {
		const char c = s.charAt(i);
		if (c == ' ') {
			out += '+';
			continue;
		}
		if (isUnreservedFormChar(c)) {
			out += c;
			continue;
		}
		char buf[4];
		snprintf(buf, sizeof(buf), "%%%02X", static_cast<unsigned char>(c));
		out += buf;
	}
	return out;
}

static int hexVal(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
	if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
	return -1;
}

static String percentDecode(const String &s) {
	String out;
	out.reserve(s.length());
	for (uint32_t i = 0; i < s.length(); ++i) {
		const char c = s.charAt(i);
		if (c == '%' && i + 2 < s.length()) {
			const int a = hexVal(s.charAt(i + 1));
			const int b = hexVal(s.charAt(i + 2));
			if (a >= 0 && b >= 0) {
				out += static_cast<char>((a << 4) | b);
				i += 2;
				continue;
			}
		}
		out += c;
	}
	return out;
}

static String resolveDuckDuckGoResultHref(const String &href) {
	String h = href;
	if (h.startsWith("//")) {
		h = String("https:") + h;
	} else if (h.startsWith("/")) {
		h = String("https://duckduckgo.com") + h;
	}

	const int uddgPos = h.indexOf("uddg=");
	if (uddgPos >= 0) {
		const int start = uddgPos + 5;
		int end = h.indexOf('&', start);
		if (end < 0) end = h.length();
		String enc = h.substring(start, end);
		String dec = percentDecode(enc);
		dec.trim();
		if (dec.startsWith("http://") || dec.startsWith("https://")) {
			return dec;
		}
	}

	return h;
}

static String truncateForMenu(const String &s, uint8_t maxChars) {
	String out = s;
	out.trim();
	if (out.length() <= maxChars) {
		return out;
	}
	if (maxChars <= 3) {
		return out.substring(0, maxChars);
	}
	return out.substring(0, maxChars - 3) + "...";
}

} // namespace

void Watchy::showGoogleSearch() {
	guiState = APP_STATE;

	// Simple web search opener via TextBrowser. Uses the uniform target editor
	// (space-first, ACCEPTx2 on space to finish).
	char query[256] = {0};
	if (!NetUtils::editTarget(*this, query, sizeof(query), "Search query")) {
		showMenu(menuIndex);
		return;
	}

	// Render a simple "searching" screen before network operations.
	UiSDK::clearScreen(*this);

	UITextSpec header{};
	header.x = 0;
	header.y = 36;
	header.font = &FreeMonoBold9pt7b;
	header.text = "DuckDuckGo";

	UIScrollableTextSpec body{};
	body.x = 0;
	body.y = 54;
	body.w = WatchyDisplay::WIDTH;
	body.h = WatchyDisplay::HEIGHT - 80;
	body.font = UiSDK::tinyMono6x8();
	body.fillBackground = false;
	body.centered = false;
	body.text = String("Searching for:\n") + query;
	body.firstLine = 0;
	body.maxLines = 8;
	body.lineHeight = 14;

	UIAppSpec app{};
	app.texts = &header;
	app.textCount = 1;
	app.scrollTexts = &body;
	app.scrollTextCount = 1;
	UiSDK::renderApp(display, app);
	display.display(true);

	if (!NetUtils::ensureWiFi(*this, 1, 8000)) {
		showTextBrowser(String("https://html.duckduckgo.com/"));
		return;
	}

	String url = "https://html.duckduckgo.com/html/";
	String form = String("q=") + urlEncodeFormComponent(String(query)) + "&b=";

	NetResponse resp = NetUtils::httpPostForm(url,
									 form,
									 "https://html.duckduckgo.com",
									 "https://html.duckduckgo.com/",
									 "es-ES,es;q=0.9,en;q=0.8",
									 30000,
									 true);

	if (resp.httpCode != HTTP_CODE_OK) {
		// Fall back: open DDG HTML home so user can at least browse.
		showTextBrowser(String("https://html.duckduckgo.com/"));
		return;
	}

	// Parse results.
	static constexpr uint8_t kMaxResults = 12;
	String titles[kMaxResults];
	String links[kMaxResults];
	uint8_t found = 0;

	const String &html = resp.body;
	int pos = 0;
	while (found < kMaxResults) {
		pos = html.indexOf("result__a", pos);
		if (pos < 0) break;
		int tagStart = html.lastIndexOf("<a", pos);
		if (tagStart < 0) { pos += 9; continue; }
		int hrefPos = html.indexOf("href=\"", tagStart);
		if (hrefPos < 0) { pos += 9; continue; }
		hrefPos += 6;
		int hrefEnd = html.indexOf('"', hrefPos);
		if (hrefEnd < 0) { pos += 9; continue; }
		String href = html.substring(hrefPos, hrefEnd);

		int textStart = html.indexOf('>', hrefEnd);
		if (textStart < 0) { pos = hrefEnd; continue; }
		int textEnd = html.indexOf("</a>", textStart);
		if (textEnd < 0) { pos = hrefEnd; continue; }
		String title = html.substring(textStart + 1, textEnd);
		title = htmlDecodeMinimal(stripTags(title));

		String target = resolveDuckDuckGoResultHref(href);
		if (title.length() == 0 || target.length() == 0) {
			pos = textEnd;
			continue;
		}

		titles[found] = title;
		links[found] = target;
		found++;
		pos = textEnd;
	}

	if (found == 0) {
		showTextBrowser(String("https://html.duckduckgo.com/html/?q=") + urlEncodeFormComponent(String(query)));
		return;
	}

	// Menu UI for results.
	static UIMenuItemSpec items[kMaxResults];
	for (uint8_t i = 0; i < found; ++i) {
		items[i].label = truncateForMenu(titles[i], 24);
	}

	int8_t selected = 0;
	while (true) {
		UiSDK::initScreen(*this);


		UITextSpec h{};
		h.x = 0;
		h.y = 36;
		h.font = &FreeMonoBold9pt7b;
		h.text = "DDG Results";

		UIMenuSpec menu{};
		menu.x = 0;
		menu.y = 54;
		menu.w = WatchyDisplay::WIDTH;
		menu.itemHeight = MENU_HEIGHT;
		menu.font = UiSDK::tinyMono6x8();
		menu.items = items;
		menu.itemCount = found;
		menu.selectedIndex = selected;
		menu.visibleCount = 5;
		menu.startIndex = UiTemplates::calcMenuStartIndex(static_cast<uint8_t>(selected), 5, found);

		UIAppSpec a{};
		a.texts = &h;
		a.textCount = 1;
		a.menus = &menu;
		a.menuCount = 1;
		UiSDK::renderControlsRow(display, "BACK", "UP", "OPEN", "DOWN");
		UiSDK::renderApp(display, a);

		if (UiSDK::buttonPressed(BACK_BTN_PIN)) {
			showMenu(menuIndex);
			return;
		}
		if (UiSDK::buttonPressed(UP_BTN_PIN)) {
			selected = (selected > 0) ? (selected - 1) : static_cast<int8_t>(found - 1);
			continue;
		}
		if (UiSDK::buttonPressed(DOWN_BTN_PIN)) {
			selected = (selected + 1 >= static_cast<int8_t>(found)) ? 0 : (selected + 1);
			continue;
		}
		if (UiSDK::buttonPressed(MENU_BTN_PIN)) {
			showTextBrowser(links[selected]);
			return;
		}
	}
}

#endif
