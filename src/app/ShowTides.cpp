#include "WatchyUi.h"
#include "Watchy.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include <Tides.h>
#pragma GCC diagnostic pop
#include <float.h>
#include "AppDisplay.h"

RTC_DATA_ATTR int tideStationIndex = -1;

namespace {

constexpr int stationCount = sizeof(REGISTRY) / sizeof(REGISTRY[0]);
const StationDef *sortedStations[stationCount];
bool stationsSorted = false;

void sortStations() {
  if (stationsSorted) {
    return;
  }

  for (int index = 0; index < stationCount; index++) {
    sortedStations[index] = REGISTRY[index];
  }

  for (int index = 1; index < stationCount; index++) {
    const StationDef *station = sortedStations[index];
    int insertionIndex = index;
    while (insertionIndex > 0 &&
           strcmp(sortedStations[insertionIndex - 1]->id, station->id) > 0) {
      sortedStations[insertionIndex] = sortedStations[insertionIndex - 1];
      insertionIndex--;
    }
    sortedStations[insertionIndex] = station;
  }

  stationsSorted = true;
}

void selectInitialStation() {
  sortStations();
  if (tideStationIndex >= 0 && tideStationIndex < stationCount) {
    return;
  }

  tideStationIndex = 0;
  for (int index = 0; index < stationCount; index++) {
    if (strcmp(sortedStations[index]->id, TIDE_STATION) == 0) {
      tideStationIndex = index;
      return;
    }
  }
}

void beginTidesDisplay(int stationIndex) {
  beginAppDisplay("TIDES");
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(WatchyUi::Theme::foreground());
  Watchy::display.setCursor(8, 37);
  Watchy::display.print(sortedStations[stationIndex]->id);
  Watchy::display.setCursor(157, 37);
  Watchy::display.print(stationIndex + 1);
  Watchy::display.print('/');
  Watchy::display.print(stationCount);
}

void drawTideCurve(const TideInfo &tide, double minimum, double maximum) {
  constexpr WatchyUi::Bounds chart{12, 45, 176, 70};
  WatchyUi::GrayPaint::fillRoundRect(
      chart, 4, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  const uint16_t foreground = WatchyUi::Theme::foreground();
  for (uint8_t grid = 1; grid < 3; grid++) {
    int16_t y = chart.y + grid * chart.height / 3;
    WatchyUi::GrayPaint::line(chart.x + 4, y, chart.x + chart.width - 5, y,
                               WatchyUi::Theme::tone(WatchyUi::ToneRole::Separator));
  }
  if (!tide.amplitudeCalculated || maximum <= minimum) return;
  int16_t previousX = 0;
  int16_t previousY = 0;
  for (int index = 0; index < TIDE_AMPLITUDE_SAMPLES; index++) {
    double amplitude = tide.amplitudePoints[index];
    float ratio = constrain(static_cast<float>((amplitude - minimum) /
                                                (maximum - minimum)),
                            0.0f, 1.0f);
    int16_t x = chart.x + 4 +
                index * (chart.width - 9) / (TIDE_AMPLITUDE_SAMPLES - 1);
    int16_t y = chart.y + chart.height - 5 -
                static_cast<int16_t>(ratio * (chart.height - 10));
    if (index > 0) {
      Watchy::display.drawLine(previousX, previousY, x, y, foreground);
      WatchyUi::GrayPaint::line(x, y, x, chart.y + chart.height - 5,
                                 WatchyUi::Theme::tone(WatchyUi::ToneRole::Selection));
    }
    previousX = x;
    previousY = y;
  }
}

void drawTideEvent(const TideInfo &tide, int index, int16_t y) {
  if (index >= tide.numEvents) {
    AppVisual::drawDataRow(y, "EVENT", "--");
    return;
  }
  int totalMinutes = static_cast<int>(tide.getEventTime(index) * 60.0f + 0.5f);
  totalMinutes %= 24 * 60;
  char label[8];
  char detail[16];
  snprintf(label, sizeof(label), "%s", tide.events[index].isPeak ? "HIGH" : "LOW");
  snprintf(detail, sizeof(detail), "%02d:%02d  %.2f m", totalMinutes / 60,
           totalMinutes % 60, tide.events[index].amplitude);
  AppVisual::drawDataRow(y, label, detail, tide.events[index].isPeak);
}

void drawTideInfo(const TideInfo &tide) {
  double minimum = DBL_MAX;
  double maximum = -DBL_MAX;
  if (tide.amplitudeCalculated) {
    for (int index = 0; index < TIDE_AMPLITUDE_SAMPLES; index++) {
      const double amplitude = tide.amplitudePoints[index];
      if (amplitude < minimum) minimum = amplitude;
      if (amplitude > maximum) maximum = amplitude;
    }
  }
  if (!tide.amplitudeCalculated) {
    AppVisual::drawEmptyState({8, 49, 184, 112}, "TIDE DATA UNAVAILABLE",
                              "No amplitude samples from this station");
    WatchyUi::Widget::footer("UP/DOWN STATION  BACK EXIT");
    finishAppDisplay();
    return;
  }
  if (maximum - minimum < 0.05) {
    minimum -= 0.05;
    maximum += 0.05;
  }
  drawTideCurve(tide, minimum, maximum);
  char coefficients[16];
  snprintf(coefficients, sizeof(coefficients), "COEFF %u / %u",
           tide.morningCoefficient, tide.afternoonCoefficient);
  WatchyUi::Canvas::centeredText({0, 117, 200, 12}, coefficients, 1,
                                 WatchyUi::Theme::foreground());
  drawTideEvent(tide, 0, 140);
  drawTideEvent(tide, 1, 158);
  drawTideEvent(tide, 2, 176);
  WatchyUi::Widget::footer("UP/DOWN STATION  BACK EXIT");
  finishAppDisplay();
}

void drawTides(int stationIndex, const tmElements_t &date) {
  beginTidesDisplay(stationIndex);

  if (!setStation(sortedStations[stationIndex]->id, 0.005)) {
    AppVisual::drawEmptyState({8, 49, 184, 112}, "STATION UNAVAILABLE",
                              "No harmonic data for this station");
    WatchyUi::Widget::footer("UP/DOWN STATION  BACK EXIT");
    finishAppDisplay();
    return;
  }

  TideInfo tide = tides(tmYearToCalendar(date.Year), date.Month, date.Day);
  drawTideInfo(tide);
}

} // namespace

void showTidesImpl(Watchy *watchy) {
  tmElements_t currentTime;
  if (watchy != nullptr) {
    watchy->RTC.read(watchy->currentTime);
    currentTime = watchy->currentTime;
  } else {
    WatchySdk::RTC.read(WatchySdk::currentTime);
    currentTime = WatchySdk::currentTime;
  }
  selectInitialStation();

  WatchyUi::Input::begin();
  drawTides(tideStationIndex, currentTime);

  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      if (watchy != nullptr) {
        watchy->showMenu(menuIndex, false);
      } else {
        WatchySdk::showMenu(menuIndex, false);
      }
      return;
    }

    if (event == WatchyUi::Event::UP) {
      tideStationIndex = (tideStationIndex + stationCount - 1) % stationCount;
      drawTides(tideStationIndex, currentTime);
    } else if (event == WatchyUi::Event::DOWN) {
      tideStationIndex = (tideStationIndex + 1) % stationCount;
      drawTides(tideStationIndex, currentTime);
    }
  }
}

void Watchy::showTides() { showTidesImpl(this); }

void WatchySdk::showTides() { showTidesImpl(nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderTidesPreview(uint8_t view, const tmElements_t &fixedTime) {
  selectInitialStation();
  if (view != 0) {
    beginTidesDisplay(tideStationIndex);
    AppVisual::drawEmptyState({8, 49, 184, 112}, "STATION UNAVAILABLE",
                              "No harmonic data for this station");
    WatchyUi::Widget::footer("UP/DOWN STATION  BACK EXIT");
    finishAppDisplay();
    return;
  }
  TideInfo tide;
  tide.numEvents = 4;
  tide.events[0] = {4.86, 2.70f, true};
  tide.events[1] = {0.74, 8.95f, false};
  tide.events[2] = {5.04, 15.13f, true};
  tide.events[3] = {0.68, 21.35f, false};
  tide.morningCoefficient = 68;
  tide.afternoonCoefficient = 71;
  tide.amplitudeCalculated = true;
  for (int index = 0; index < TIDE_AMPLITUDE_SAMPLES; index++) {
    tide.amplitudePoints[index] = 2.8;
  }
  tide.amplitudePoints[0] = 0.68;
  tide.amplitudePoints[1] = 5.04;
  tmElements_t noon = fixedTime;
  noon.Hour = 12;
  noon.Minute = 0;
  noon.Second = 0;
  tide.epoch = makeTime(noon);

  beginTidesDisplay(tideStationIndex);
  drawTideInfo(tide);
}

} // namespace WatchyDemo
#endif
