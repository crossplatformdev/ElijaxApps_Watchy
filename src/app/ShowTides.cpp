#include <Watchy.h>
#include <Tides.h>
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

void printEvent(const TideInfo &tide, int index, int yPosition) {
  Watchy::display.setCursor(0, yPosition);
  if (index >= tide.numEvents) {
    Watchy::display.println("--");
    return;
  }

  int totalMinutes = (int)(tide.getEventTime(index) * 60.0f + 0.5f);
  totalMinutes %= 24 * 60;
  Watchy::display.print(tide.events[index].isPeak ? "HIGH " : "LOW  ");
  printTwoDigits(totalMinutes / 60);
  Watchy::display.print(':');
  printTwoDigits(totalMinutes % 60);
  Watchy::display.print(' ');
  Watchy::display.print(tide.events[index].amplitude, 2);
  Watchy::display.println('m');
}

void beginTidesDisplay(int stationIndex) {
  beginAppDisplay("TIDES");
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(0, 27);
  Watchy::display.print(stationIndex + 1);
  Watchy::display.print('/');
  Watchy::display.print(stationCount);
  Watchy::display.print(' ');
  Watchy::display.println(sortedStations[stationIndex]->id);
}

void drawTideInfo(const TideInfo &tide) {
  for (int index = 0; index < 4; index++) {
    printEvent(tide, index, 45 + index * 18);
  }

  Watchy::display.setCursor(0, 120);
  Watchy::display.print("Coefficients: ");
  Watchy::display.print(tide.morningCoefficient);
  Watchy::display.print('/');
  Watchy::display.println(tide.afternoonCoefficient);

  double minimum = DBL_MAX;
  double maximum = -DBL_MAX;
  if (tide.amplitudeCalculated) {
    for (int index = 0; index < TIDE_AMPLITUDE_SAMPLES; index++) {
      const double amplitude = tide.amplitudePoints[index];
      if (amplitude < minimum) minimum = amplitude;
      if (amplitude > maximum) maximum = amplitude;
    }
  }

  Watchy::display.setCursor(0, 138);
  Watchy::display.print("Samples: ");
  Watchy::display.println(tide.amplitudeCalculated ? TIDE_AMPLITUDE_SAMPLES : 0);
  Watchy::display.setCursor(0, 156);
  Watchy::display.print("Range: ");
  if (tide.amplitudeCalculated) {
    Watchy::display.print(minimum, 2);
    Watchy::display.print(" - ");
    Watchy::display.print(maximum, 2);
    Watchy::display.println('m');
  } else {
    Watchy::display.println("unavailable");
  }

  Watchy::display.setCursor(0, 174);
  Watchy::display.print("Epoch: ");
  Watchy::display.println((uint32_t)tide.epoch);
  finishAppDisplay();
}

void drawTides(int stationIndex, const tmElements_t &date) {
  beginTidesDisplay(stationIndex);

  if (!setStation(sortedStations[stationIndex]->id, 0.005)) {
    Watchy::display.setCursor(0, 50);
    Watchy::display.println("Station unavailable");
    finishAppDisplay();
    return;
  }

  TideInfo tide = tides(tmYearToCalendar(date.Year), date.Month, date.Day);
  drawTideInfo(tide);
}

} // namespace

void Watchy::showTides() {
  RTC.read(currentTime);
  selectInitialStation();

  WatchyUi::Input::begin();
  drawTides(tideStationIndex, currentTime);

  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      showMenu(menuIndex, false);
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

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderTidesPreview(const tmElements_t &fixedTime) {
  selectInitialStation();
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