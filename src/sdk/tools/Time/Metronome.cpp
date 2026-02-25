#include "WatchyUi.h"
#include "Watchy.h"

#include <stddef.h>

#include "AppDisplay.h"
#include "MetronomeEngine.h"
#include "TimeSupport.h"
#include "TimeToolApps.h"
#include "WatchyStorage.h"

RTC_DATA_ATTR uint16_t metronomeBpm = 100;
RTC_DATA_ATTR uint8_t metronomeAccentEvery = 4;
RTC_DATA_ATTR bool metronomeSettingsLoaded = false;

namespace {

constexpr const char *alarmNamespace = "watchy-time";
constexpr const char *metronomeRecordKey = "metronome";
constexpr uint32_t metronomeRecordMagic = 0x4f52544dUL;
constexpr uint8_t metronomeRecordVersion = 1;
constexpr uint8_t maximumAccentEvery = 32;

struct MetronomeRecord {
  uint32_t magic;
  uint16_t size;
  uint8_t version;
  uint8_t accentEvery;
  uint16_t bpm;
  uint8_t reserved[2];
  uint32_t checksum;
};

static_assert(sizeof(MetronomeRecord) == 16,
              "Metronome record layout changed");

void updateMetronomeChecksum(MetronomeRecord &record) {
  record.checksum = WatchySdk::recordChecksum(
      &record, offsetof(MetronomeRecord, checksum));
}

bool validMetronomeRecord(const MetronomeRecord &record) {
  return record.magic == metronomeRecordMagic &&
         record.size == sizeof(record) &&
         record.version == metronomeRecordVersion &&
         record.bpm >= WatchyMetronome::minimumBpm &&
         record.bpm <= WatchyMetronome::maximumBpm &&
         record.accentEvery >= 1 &&
         record.accentEvery <= maximumAccentEvery &&
         record.checksum == WatchySdk::recordChecksum(
             &record, offsetof(MetronomeRecord, checksum));
}

MetronomeRecord makeMetronomeRecord(uint16_t bpm, uint8_t accentEvery) {
  MetronomeRecord record{};
  record.magic = metronomeRecordMagic;
  record.size = sizeof(record);
  record.version = metronomeRecordVersion;
  record.bpm = bpm;
  record.accentEvery = accentEvery;
  updateMetronomeChecksum(record);
  return record;
}

void loadMetronomeSettings(uint16_t &bpm, uint8_t &accentEvery) {
  if (!metronomeSettingsLoaded ||
      metronomeBpm < WatchyMetronome::minimumBpm ||
      metronomeBpm > WatchyMetronome::maximumBpm ||
      metronomeAccentEvery == 0 ||
      metronomeAccentEvery > maximumAccentEvery) {
    MetronomeRecord record{};
    if (WatchySdk::Storage::read(alarmNamespace, metronomeRecordKey,
                                &record, sizeof(record)) &&
        validMetronomeRecord(record)) {
      metronomeBpm = record.bpm;
      metronomeAccentEvery = record.accentEvery;
    } else {
      metronomeBpm = 100;
      metronomeAccentEvery = 4;
      record = makeMetronomeRecord(metronomeBpm,
                                   metronomeAccentEvery);
      WatchySdk::Storage::write(alarmNamespace, metronomeRecordKey,
                                &record, sizeof(record));
    }
    metronomeSettingsLoaded = true;
  }
  bpm = metronomeBpm;
  accentEvery = metronomeAccentEvery;
}

bool saveMetronomeSettings(uint16_t bpm, uint8_t accentEvery) {
  metronomeBpm = bpm;
  metronomeAccentEvery = accentEvery;
  metronomeSettingsLoaded = true;
  MetronomeRecord record = makeMetronomeRecord(bpm, accentEvery);
  return WatchySdk::Storage::write(alarmNamespace, metronomeRecordKey,
                                  &record, sizeof(record));
}

} // namespace

namespace WatchyTimeTools {
namespace {

void showMetronomeSaveWarning() {
  WatchyUi::Feedback::showMessage(
      "METRONOME", "Setting changed for this session but was not saved.",
      WatchyUi::MessageKind::WARNING, "SELECT CONTINUE");
  WatchyUi::Input::wait();
}

enum MetronomeOption : uint8_t {
  METRONOME_BPM,
  METRONOME_ACCENT,
  METRONOME_START,
  METRONOME_OPTION_COUNT
};

constexpr WatchyUi::Bounds metronomeStatusBounds{12, 150, 176, 18};

const char *tempoMarking(uint16_t bpm) {
  if (bpm < 40) return "GRAVE";
  if (bpm < 60) return "LARGO";
  if (bpm < 76) return "ADAGIO";
  if (bpm < 108) return "ANDANTE";
  if (bpm < 120) return "MODERATO";
  if (bpm < 156) return "ALLEGRO";
  if (bpm < 200) return "PRESTO";
  return "PRESTISSIMO";
}

struct MetronomeSelectorContext {
  char bpm[8];
  char accent[16];
};

const char *metronomeOptionLabel(uint8_t index, const void *) {
  static const char *const labels[] = {"BPM", "ACCENT", "START"};
  return index < METRONOME_OPTION_COUNT ? labels[index] : "";
}

const char *metronomeOptionDetail(uint8_t index, const void *rawContext) {
  const MetronomeSelectorContext &context =
      *static_cast<const MetronomeSelectorContext *>(rawContext);
  const char *detail = "";
  switch (index) {
  case METRONOME_BPM: detail = context.bpm; break;
  case METRONOME_ACCENT: detail = context.accent; break;
  default: break;
  }
  return detail;
}

void drawMetronomeSelector(uint16_t bpm, uint8_t accentEvery,
                           uint8_t selected) {
  MetronomeSelectorContext context{};
  snprintf(context.bpm, sizeof(context.bpm), "%u", bpm);
  snprintf(context.accent, sizeof(context.accent), "EVERY %u", accentEvery);
  WatchyUi::ListModel model{
      "METRONOME", metronomeOptionLabel, metronomeOptionDetail, &context,
      "UP/DOWN  SELECT  BACK EXIT", METRONOME_OPTION_COUNT, selected,
      METRONOME_OPTION_COUNT, -1, true, false};
  WatchyUi::ListView::draw(model);
  WatchyUi::Screen::present();
}

void updateMetronomeSelector(uint16_t bpm, uint8_t accentEvery,
                             uint8_t previous, uint8_t selected) {
  MetronomeSelectorContext context{};
  snprintf(context.bpm, sizeof(context.bpm), "%u", bpm);
  snprintf(context.accent, sizeof(context.accent), "EVERY %u", accentEvery);
  WatchyUi::ListModel model{
      "METRONOME", metronomeOptionLabel, metronomeOptionDetail, &context,
      "UP/DOWN  SELECT  BACK EXIT", METRONOME_OPTION_COUNT, selected,
      METRONOME_OPTION_COUNT, -1, true, false};
  WatchyUi::ListView::presentSelectionChange(model, previous);
}

uint16_t editMetronomeValue(const char *title, uint16_t value,
                            uint16_t minimum, uint16_t maximum) {
  uint16_t candidate = value;
  while (true) {
    char text[8];
    snprintf(text, sizeof(text), "%u", candidate);
    WatchyUi::ValueModel model{
        title, text, "PAUSED", nullptr,
        "UP/DOWN 1  SELECT OK  BACK CANCEL"};
    WatchyUi::ValueView::draw(model);
    WatchyUi::Screen::present();
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::MENU) {
      return candidate;
    }
    if (event == WatchyUi::Event::BACK) {
      return value;
    }
    if (event == WatchyUi::Event::UP) {
      candidate = WatchyUi::Selector::step(candidate, 1, minimum, maximum);
    } else if (event == WatchyUi::Event::DOWN) {
      candidate = WatchyUi::Selector::step(candidate, -1, minimum, maximum);
    }
  }
}

uint32_t metronomeMeasure(uint32_t beat, uint8_t accentEvery) {
  return beat == 0 ? 0 : (beat - 1) / accentEvery + 1;
}

void drawMetronomeStatus(uint32_t beat, uint8_t accentEvery) {
  char status[32];
  uint32_t measure = metronomeMeasure(beat, accentEvery);
  if (measure == 0) {
    snprintf(status, sizeof(status), "HAPTIC  STARTING");
  } else {
    snprintf(status, sizeof(status), "HAPTIC  MEASURE %lu",
             static_cast<unsigned long>(measure));
  }
  WatchyUi::GrayPaint::fillRoundRect(
      metronomeStatusBounds, 3,
      WatchyUi::Theme::tone(WatchyUi::ToneRole::SurfaceRaised));
  WatchyUi::Canvas::centeredText(
      metronomeStatusBounds, status, 1, WatchyUi::Theme::foreground());
}

void drawMetronomeAccentPattern(uint8_t accentEvery) {
  uint16_t foreground = WatchyUi::Theme::foreground();
  char captionText[20];
  snprintf(captionText, sizeof(captionText), "ACCENT EVERY %u", accentEvery);
  WatchyUi::Canvas::centeredText({4, 114, 192, 14}, captionText, 1, foreground);

  constexpr int16_t availableWidth = 160;
  constexpr int16_t gap = 2;
  int16_t segmentWidth =
      (availableWidth - gap * (accentEvery - 1)) / accentEvery;
  segmentWidth = max<int16_t>(2, min<int16_t>(12, segmentWidth));
  int16_t patternWidth = accentEvery * segmentWidth +
                         (accentEvery - 1) * gap;
  int16_t x = (DISPLAY_WIDTH - patternWidth) / 2;
  for (uint8_t index = 0; index < accentEvery; index++) {
    bool accent = index + 1 == accentEvery;
    if (accent) {
      Watchy::display.fillRect(x, 132, segmentWidth, 12, foreground);
    } else {
      Watchy::display.drawRect(x, 135, segmentWidth, 9, foreground);
    }
    x += segmentWidth + gap;
  }
}

void drawMetronomeRunning(uint16_t bpm, uint8_t accentEvery, uint32_t beat) {
  uint16_t foreground = WatchyUi::Theme::foreground();
  WatchyUi::Screen::begin("METRONOME");
  WatchyUi::Widget::separator();
  WatchyUi::GrayPaint::fillRoundRect(
      WatchyUi::Bounds{4, 29, 192, 75}, 4,
      WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));

  char value[8];
  snprintf(value, sizeof(value), "%u", bpm);
  WatchyUi::Canvas::centeredText(
      WatchyUi::Bounds{8, 34, 110, 48}, value, 4, foreground);
  WatchyUi::Canvas::centeredText(
      WatchyUi::Bounds{8, 80, 110, 15}, "BPM", 1, foreground);
  WatchyUi::Canvas::centeredText(
      WatchyUi::Bounds{112, 82, 80, 15}, tempoMarking(bpm), 1,
      foreground);

  Watchy::display.drawCircle(154, 42, 5, foreground);
  Watchy::display.drawLine(154, 47, 136, 80, foreground);
  Watchy::display.fillCircle(133, 85, 8, foreground);
  Watchy::display.drawLine(124, 96, 184, 96, foreground);

  WatchyUi::GrayPaint::fillRoundRect(
      WatchyUi::Bounds{4, 109, 192, 63}, 4,
      WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  drawMetronomeAccentPattern(accentEvery);
  drawMetronomeStatus(beat, accentEvery);
  WatchyUi::Widget::footer("SELECT STOP  BACK EXIT");
  WatchyUi::Screen::present();
}

void updateMetronomeRunning(uint32_t beat, uint8_t accentEvery) {
  WatchyUi::GrayPaint::fillRect(
      metronomeStatusBounds,
      WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  drawMetronomeStatus(beat, accentEvery);
  WatchyUi::Screen::present(metronomeStatusBounds);
}

} // namespace

void runMetronome() {
  configureButtons();
  uint16_t bpm;
  uint8_t accentEvery;
  loadMetronomeSettings(bpm, accentEvery);
  uint32_t beat = 0;
  uint32_t displayedMeasure = 0;
  uint8_t selected = METRONOME_BPM;
  bool running = false;
  uint32_t lastRefresh = millis();
  drawMetronomeSelector(bpm, accentEvery, selected);
  while (true) {
    uint32_t waitMs = UINT32_MAX;
    if (running) {
      uint32_t sinceRefresh = millis() - lastRefresh;
      waitMs =
          sinceRefresh < WatchyUi::Screen::liveViewRefreshIntervalMs
              ? WatchyUi::Screen::liveViewRefreshIntervalMs - sinceRefresh
              : 0;
    }
    WatchyUi::Event event = running
                                ? WatchyUi::Input::waitScheduled(waitMs)
                                : WatchyUi::Input::wait(waitMs);
    if (event == WatchyUi::Event::BACK) {
      if (running) {
        WatchyMetronome::stop();
      }
      return;
    }
    if (running) {
      if (event == WatchyUi::Event::MENU) {
        WatchyMetronome::stop();
        running = false;
        drawMetronomeSelector(bpm, accentEvery, selected);
        lastRefresh = millis();
      }
    } else if (event == WatchyUi::Event::UP) {
      uint8_t previous = selected;
      selected = WatchyUi::ListView::previous(selected,
                                               METRONOME_OPTION_COUNT);
      updateMetronomeSelector(bpm, accentEvery, previous, selected);
      lastRefresh = millis();
    } else if (event == WatchyUi::Event::DOWN) {
      uint8_t previous = selected;
      selected = WatchyUi::ListView::next(selected, METRONOME_OPTION_COUNT);
      updateMetronomeSelector(bpm, accentEvery, previous, selected);
      lastRefresh = millis();
    } else if (event == WatchyUi::Event::MENU) {
      if (selected == METRONOME_BPM) {
        uint16_t previousBpm = bpm;
        bpm = editMetronomeValue("BPM", bpm, 30, 240);
        if (bpm != previousBpm &&
            !saveMetronomeSettings(bpm, accentEvery)) {
          showMetronomeSaveWarning();
        }
        drawMetronomeSelector(bpm, accentEvery, selected);
      } else if (selected == METRONOME_ACCENT) {
        uint8_t previousAccentEvery = accentEvery;
        accentEvery = static_cast<uint8_t>(
            editMetronomeValue("ACCENT", accentEvery, 1, 32));
        if (accentEvery != previousAccentEvery &&
            !saveMetronomeSettings(bpm, accentEvery)) {
          showMetronomeSaveWarning();
        }
        drawMetronomeSelector(bpm, accentEvery, selected);
      } else {
        running = WatchyMetronome::start(bpm, accentEvery);
        if (running) {
          beat = 0;
          displayedMeasure = 0;
          drawMetronomeRunning(bpm, accentEvery, beat);
        } else {
          WatchyUi::Feedback::showMessage(
              "METRONOME", "Unable to start the timing worker.",
              WatchyUi::MessageKind::ERROR, "BACK");
          WatchyUi::Input::wait();
          drawMetronomeSelector(bpm, accentEvery, selected);
        }
      }
      lastRefresh = millis();
    }
    if (running && millis() - lastRefresh >=
        WatchyUi::Screen::liveViewRefreshIntervalMs) {
      WatchyMetronome::Snapshot state = WatchyMetronome::snapshot();
      beat = state.beat;
      uint32_t measure = metronomeMeasure(beat, accentEvery);
      if (measure != displayedMeasure) {
        updateMetronomeRunning(beat, accentEvery);
        displayedMeasure = measure;
      }
      lastRefresh = millis();
    }
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderMetronomePreview(uint8_t view) {
  if (view == 0) {
    drawMetronomeSelector(120, 4, METRONOME_BPM);
  } else {
    drawMetronomeRunning(120, 4, 7);
  }
}
#endif

} // namespace WatchyTimeTools
