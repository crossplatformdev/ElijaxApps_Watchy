#include <Watchy.h>
#include "AppDisplay.h"
#include "EmergencyProfile.h"
#include "NetworkAppCommon.h"

namespace {

enum HealthcareTool : uint8_t {
  EMERGENCY_PLATE,
  MEDICAL_ID,
  ICE_CONTACT,
  BLOOD_TYPE,
  ALLERGIES,
  MEDICATIONS,
  CONDITIONS,
  EDIT_MEDICAL_ID,
  HEALTHCARE_TOOL_COUNT
};

String clipped(const String &value, size_t length) {
  if (value.length() <= length) {
    return value;
  }
  return value.substring(0, length - 3) + "...";
}

void useCompactText(int16_t x = 3, int16_t y = 32) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(x, y);
}

void printField(const char *label, const String &value, int16_t y,
                size_t maximum = 21) {
  Watchy::display.setCursor(3, y);
  Watchy::display.print(label);
  Watchy::display.setCursor(62, y);
  Watchy::display.println(clipped(value, maximum).c_str());
}

void drawEmergencyPlate(const EmergencyProfile::Data &profile) {
  beginAppDisplay("UN DOG PLATE");
  uint16_t foreground = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
  Watchy::display.drawRect(2, 25, 196, 169, foreground);
  useCompactText(8, 42);
  Watchy::display.setTextSize(2);
  Watchy::display.println(clipped(profile.name, 15));
  Watchy::display.setTextSize(1);
  printField("ID", profile.identifier, 75);
  printField("NATION", profile.country, 93);
  printField("BIRTH", profile.birthDate, 111);
  printField("BLOOD", profile.bloodType, 129);
  printField("ICE", profile.iceName, 151);
  printField("TEL", profile.icePhone, 169);
  Watchy::display.setCursor(8, 187);
  Watchy::display.print("LOCAL MEDICAL IDENTIFICATION");
  finishAppDisplay();
}

void drawMedicalId(const EmergencyProfile::Data &profile) {
  beginAppDisplay("MEDICAL ID");
  useCompactText();
  printField("NAME", profile.name, 35);
  printField("BLOOD", profile.bloodType, 55);
  printField("ALLERGY", profile.allergies, 75);
  printField("CONDITION", profile.conditions, 95);
  printField("MEDS", profile.medications, 115);
  printField("ICE", profile.iceName, 145);
  printField("PHONE", profile.icePhone, 165);
  Watchy::display.setCursor(3, 190);
  Watchy::display.print("VERIFY WITH PATIENT / CLINICIAN");
  finishAppDisplay();
}

void drawSingleField(const char *title, const char *label,
                     const String &value, const char *footer) {
  beginAppDisplay(title);
  useCompactText(5, 48);
  Watchy::display.println(label);
  Watchy::display.setTextSize(2);
  Watchy::display.setCursor(5, 84);
  Watchy::display.println(clipped(value, 15));
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(5, 150);
  Watchy::display.println(footer);
  finishAppDisplay();
}

void drawIce(const EmergencyProfile::Data &profile) {
  beginAppDisplay("ICE CONTACT");
  useCompactText(5, 50);
  Watchy::display.println("IN CASE OF EMERGENCY");
  Watchy::display.setTextSize(2);
  Watchy::display.setCursor(5, 88);
  Watchy::display.println(clipped(profile.iceName, 15));
  Watchy::display.setCursor(5, 126);
  Watchy::display.println(clipped(profile.icePhone, 15));
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(5, 170);
  Watchy::display.println("THIS WATCH CANNOT PLACE CALLS");
  finishAppDisplay();
}

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
      true};
  WatchyUi::ListView::draw(model);
  WatchyUi::Screen::present();
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
      selected = WatchyUi::ListView::previous(
          selected, EmergencyProfile::FIELD_COUNT);
    } else if (event == WatchyUi::Event::DOWN) {
      selected = WatchyUi::ListView::next(
          selected, EmergencyProfile::FIELD_COUNT);
    } else if (event == WatchyUi::Event::SELECT) {
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
          delay(700);
        }
      }
      WatchyUi::Input::begin();
    }
    drawProfileEditor(profile, selected);
  }
}

} // namespace

void Watchy::showHealthcareTool(uint8_t tool) {
  if (tool == EDIT_MEDICAL_ID) {
    editProfile();
    showMenu(menuIndex, false);
    return;
  }

  EmergencyProfile::Data profile;
  EmergencyProfile::load(profile);
  switch (tool) {
  case EMERGENCY_PLATE: drawEmergencyPlate(profile); break;
  case MEDICAL_ID: drawMedicalId(profile); break;
  case ICE_CONTACT: drawIce(profile); break;
  case BLOOD_TYPE:
    drawSingleField("BLOOD TYPE", "RECORDED BLOOD GROUP", profile.bloodType,
                    "Confirm before transfusion");
    break;
  case ALLERGIES:
    drawSingleField("ALLERGIES", "RECORDED ALLERGIES", profile.allergies,
                    "Verify with patient");
    break;
  case MEDICATIONS:
    drawSingleField("MEDICATIONS", "CURRENT MEDICATIONS", profile.medications,
                    "Dose details may be incomplete");
    break;
  case CONDITIONS:
    drawSingleField("CONDITIONS", "KNOWN CONDITIONS", profile.conditions,
                    "Not a clinical record");
    break;
  default: break;
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
#include "demo/GalleryFixtures.h"

namespace WatchyDemo {

void renderHealthcarePreview(uint8_t tool) {
  EmergencyProfile::Data profile = GalleryFixtures::emergencyProfile();
  switch (tool) {
  case EMERGENCY_PLATE: drawEmergencyPlate(profile); break;
  case MEDICAL_ID: drawMedicalId(profile); break;
  case ICE_CONTACT: drawIce(profile); break;
  case BLOOD_TYPE:
    drawSingleField("BLOOD TYPE", "RECORDED BLOOD GROUP", profile.bloodType,
                    "Confirm before transfusion");
    break;
  case ALLERGIES:
    drawSingleField("ALLERGIES", "RECORDED ALLERGIES", profile.allergies,
                    "Verify with patient");
    break;
  case MEDICATIONS:
    drawSingleField("MEDICATIONS", "CURRENT MEDICATIONS",
                    profile.medications, "Dose details may be incomplete");
    break;
  case CONDITIONS:
    drawSingleField("CONDITIONS", "KNOWN CONDITIONS", profile.conditions,
                    "Not a clinical record");
    break;
  case EDIT_MEDICAL_ID:
    drawProfileEditor(profile, EmergencyProfile::FIELD_BLOOD_TYPE);
    break;
  default:
    break;
  }
}

} // namespace WatchyDemo
#endif