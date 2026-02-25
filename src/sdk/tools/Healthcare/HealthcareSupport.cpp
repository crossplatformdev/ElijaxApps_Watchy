#include "WatchyUi.h"

#include "AppDisplay.h"
#include "EmergencyProfile.h"
#include "HealthcareSupport.h"
#include "NetworkAppCommon.h"

namespace WatchyHealthcareTools {
namespace {

void drawProfileEditor(const EmergencyProfile::Data &profile,
                       uint8_t selected) {
  WatchyUi::ListModel model{
      "EDIT MEDICAL ID",
      [](uint8_t index, const void *) {
        return EmergencyProfile::label(
            static_cast<EmergencyProfile::Field>(index));
      },
      [](uint8_t index, const void *context) {
        const EmergencyProfile::Data &data =
            *static_cast<const EmergencyProfile::Data *>(context);
        return EmergencyProfile::value(
                   data, static_cast<EmergencyProfile::Field>(index))
            .c_str();
      },
      &profile,
      "UP/DOWN  SELECT EDIT  BACK EXIT",
      EmergencyProfile::FIELD_COUNT,
      selected,
      WatchyUi::Theme::listVisibleRows,
      -1,
      true,
      true};
  WatchyUi::ListView::draw(model);
  WatchyUi::Screen::present();
}

void updateProfileEditor(const EmergencyProfile::Data &profile,
                         uint8_t previous, uint8_t selected) {
  WatchyUi::ListModel model{
      "EDIT MEDICAL ID",
      [](uint8_t index, const void *) {
        return EmergencyProfile::label(
            static_cast<EmergencyProfile::Field>(index));
      },
      [](uint8_t index, const void *context) {
        const EmergencyProfile::Data &data =
            *static_cast<const EmergencyProfile::Data *>(context);
        return EmergencyProfile::value(
                   data, static_cast<EmergencyProfile::Field>(index))
            .c_str();
      },
      &profile,
      "UP/DOWN  SELECT EDIT  BACK EXIT",
      EmergencyProfile::FIELD_COUNT,
      selected,
      WatchyUi::Theme::listVisibleRows,
      -1,
      true,
      true};
  WatchyUi::ListView::presentSelectionChange(model, previous);
}

} // namespace

String clipped(const String &value, size_t length) {
  if (value.length() <= length) {
    return value;
  }
  return value.substring(0, length - 3) + "...";
}

void drawSingleField(const char *title, const char *label,
                     const String &value, const char *footer) {
  WatchyUi::Screen::begin(title);
  if (value == "NOT SET") {
    AppVisual::drawEmptyState({8, 40, 184, 118}, "NOT RECORDED",
                              "Use Edit Medical ID to add this field");
  } else {
    AppVisual::drawStatusIcon({79, 35, 42, 42}, AppVisual::StatusIcon::INFO,
                              true);
    WatchyUi::Canvas::centeredText({12, 88, 176, 18}, label, 1,
                                   WatchyUi::Theme::foreground());
    WatchyUi::Widget::paragraph(value.c_str(), 15, 121, 28, 5, 10);
    WatchyUi::Canvas::centeredText({12, 173, 176, 13}, footer, 1,
                                   WatchyUi::Theme::foreground());
  }
  WatchyUi::Widget::footer("UP/DOWN SCROLL  BACK EXIT");
  WatchyUi::Screen::present();
}

void showSingleField(const char *title, const char *label,
                     const String &value, const char *footer) {
  String text = String(label) + "\n\n" + value + "\n\n" + footer;
  WatchyUi::ScrollableTextView::show(title, text.c_str());
}

void editProfile() {
  EmergencyProfile::Data profile;
  EmergencyProfile::load(profile);
  uint8_t selected = 0;
  WatchyUi::Input::begin();
  drawProfileEditor(profile, selected);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    if (event == WatchyUi::Event::UP) {
      uint8_t previous = selected;
      selected = WatchyUi::ListView::previous(
          selected, EmergencyProfile::FIELD_COUNT);
      updateProfileEditor(profile, previous, selected);
      continue;
    } else if (event == WatchyUi::Event::DOWN) {
      uint8_t previous = selected;
      selected = WatchyUi::ListView::next(
          selected, EmergencyProfile::FIELD_COUNT);
      updateProfileEditor(profile, previous, selected);
      continue;
    } else if (event == WatchyUi::Event::MENU) {
      EmergencyProfile::Field field =
          static_cast<EmergencyProfile::Field>(selected);
      String edited;
      bool accepted = NetworkApps::editText(
          EmergencyProfile::label(field),
          EmergencyProfile::value(profile, field), edited,
          EmergencyProfile::maximumLength(field));
      if (accepted) {
        if (EmergencyProfile::save(field, edited)) {
          EmergencyProfile::value(profile, field) = edited;
        } else {
          WatchyUi::Feedback::toast("SETTING NOT SAVED");
          if (WatchyUi::deepSleepDelay(700) ==
              WatchyUi::WakeupReason::BACK_PRESSED) {
            return;
          }
        }
      }
      WatchyUi::Input::begin();
    }
    drawProfileEditor(profile, selected);
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderProfileEditorPreview(const EmergencyProfile::Data &profile,
                                uint8_t view) {
  if (view == 0) {
    drawProfileEditor(profile, EmergencyProfile::FIELD_BLOOD_TYPE);
  } else {
    NetworkApps::renderTextEditorPreview("BLOOD TYPE", profile.bloodType);
  }
}
#endif

} // namespace WatchyHealthcareTools
