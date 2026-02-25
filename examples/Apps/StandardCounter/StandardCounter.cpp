#include "StandardCounter.h"
#include <WatchySdk.h>
#include <stddef.h>
#include "sdk/WatchyStorage.h"
#include "sdk/WatchyUi.h"

namespace StandardCounterApp {
namespace {

constexpr const char *storageNamespace = "counter-demo";
constexpr const char *recordKey = "settings";
constexpr uint32_t recordMagic = 0x52544e43UL;
constexpr uint8_t recordVersion = 1;

struct CounterRecord {
  uint32_t magic;
  uint16_t size;
  uint8_t version;
  uint8_t step;
  int32_t value;
  uint32_t checksum;
};

static_assert(sizeof(CounterRecord) == 16, "Counter record layout changed");

void updateChecksum(CounterRecord &record) {
  record.checksum = WatchySdk::recordChecksum(
      &record, offsetof(CounterRecord, checksum));
}

CounterRecord defaultRecord() {
  CounterRecord record{};
  record.magic = recordMagic;
  record.size = sizeof(record);
  record.version = recordVersion;
  record.step = 1;
  updateChecksum(record);
  return record;
}

bool validRecord(const CounterRecord &record) {
  return record.magic == recordMagic && record.size == sizeof(record) &&
         record.version == recordVersion && record.step >= 1 &&
         record.step <= 10 && record.value >= -9999 && record.value <= 9999 &&
         record.checksum == WatchySdk::recordChecksum(
             &record, offsetof(CounterRecord, checksum));
}

CounterRecord loadRecord() {
  CounterRecord record{};
  if (WatchySdk::Storage::read(storageNamespace, recordKey,
                              &record, sizeof(record)) &&
      validRecord(record)) {
    return record;
  }
  return defaultRecord();
}

bool commitRecord(CounterRecord &live, CounterRecord candidate) {
  updateChecksum(candidate);
  if (!WatchySdk::Storage::write(storageNamespace, recordKey,
                                 &candidate, sizeof(candidate))) {
    return false;
  }
  live = candidate;
  return true;
}

void showResult(const char *message, WatchyUi::MessageKind kind) {
  WatchyUi::Feedback::showMessage(
      "STANDARD COUNTER", message, kind, "SELECT OR BACK TO CONTINUE");
  WatchyUi::Input::begin();
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::SELECT ||
        event == WatchyUi::Event::BACK) {
      return;
    }
  }
}

void drawMenu(uint8_t selected) {
  const char *items[] = {"EDIT COUNTER", "EDIT STEP", "RESET"};
  WatchyUi::ListView::draw("STANDARD COUNTER", items, 3, selected,
                           "SELECT OPEN        BACK EXIT");
  WatchyUi::Screen::present();
}

void editNumber(CounterRecord &record, bool editingStep) {
  CounterRecord candidate = record;
  while (true) {
    char value[12];
    char status[20];
    if (editingStep) {
      snprintf(value, sizeof(value), "%u", candidate.step);
      snprintf(status, sizeof(status), "COUNTER %ld",
               static_cast<long>(candidate.value));
    } else {
      snprintf(value, sizeof(value), "%ld",
               static_cast<long>(candidate.value));
      snprintf(status, sizeof(status), "STEP %u", candidate.step);
    }
    WatchyUi::ValueModel model{
        editingStep ? "COUNTER STEP" : "COUNTER VALUE", value, status,
        editingStep ? "Choose an increment from 1 to 10."
                    : "UP and DOWN apply the configured step.",
        "UP/DOWN CHANGE SELECT SAVE BACK"};
    WatchyUi::ValueView::draw(model);
    WatchyUi::Screen::present();

    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (event == WatchyUi::Event::UP || event == WatchyUi::Event::DOWN) {
      int32_t direction = event == WatchyUi::Event::UP ? 1 : -1;
      if (editingStep) {
        candidate.step = static_cast<uint8_t>(WatchyUi::Selector::step(
            candidate.step, direction, 1, 10));
      } else {
        candidate.value = WatchyUi::Selector::step(
            candidate.value, direction * candidate.step, -9999, 9999);
      }
    } else if (event == WatchyUi::Event::SELECT) {
      if (commitRecord(record, candidate)) {
        vibMotor(40, 1);
        showResult("Counter settings saved.",
                   WatchyUi::MessageKind::SUCCESS);
        return;
      }
      showResult("The setting could not be saved.",
                 WatchyUi::MessageKind::ERROR);
    }
  }
}

void resetCounter(CounterRecord &record) {
  if (!WatchyUi::Feedback::confirm(
          "RESET COUNTER", "Reset the value and step to their defaults?")) {
    return;
  }
  CounterRecord candidate = defaultRecord();
  if (commitRecord(record, candidate)) {
    vibMotor(40, 1);
    showResult("Counter reset.", WatchyUi::MessageKind::SUCCESS);
  } else {
    showResult("The counter could not be reset.",
               WatchyUi::MessageKind::ERROR);
  }
}

} // namespace

void run() {
  CounterRecord record = loadRecord();
  uint8_t selected = 0;
  WatchyUi::Input::begin();

  while (true) {
    drawMenu(selected);
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      showMenu(menuIndex, false);
      return;
    }
    if (event == WatchyUi::Event::UP) {
      selected = WatchyUi::ListView::previous(selected, 3);
    } else if (event == WatchyUi::Event::DOWN) {
      selected = WatchyUi::ListView::next(selected, 3);
    } else if (event == WatchyUi::Event::SELECT) {
      if (selected < 2) editNumber(record, selected == 1);
      else resetCounter(record);
    }
  }
}

} // namespace StandardCounterApp