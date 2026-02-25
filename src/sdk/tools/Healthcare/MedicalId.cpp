#include "WatchyUi.h"

#include "AppDisplay.h"
#include "HealthcareSupport.h"
#include "HealthcareToolApps.h"

namespace WatchyHealthcareTools {

void showMedicalId(const EmergencyProfile::Data &profile) {
  String text = "NAME: " + profile.name +
                "\nBLOOD: " + profile.bloodType +
                "\nALLERGIES: " + profile.allergies +
                "\nCONDITIONS: " + profile.conditions +
                "\nMEDICATIONS: " + profile.medications +
                "\nICE: " + profile.iceName +
                "\nPHONE: " + profile.icePhone +
                "\n\nVerify with patient / clinician.";
  WatchyUi::ScrollableTextView::show("MEDICAL ID", text.c_str());
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderMedicalIdPreview(const EmergencyProfile::Data &profile) {
  WatchyUi::Screen::begin("MEDICAL ID");
  if (profile.name == "NOT SET") {
    AppVisual::drawEmptyState({8, 40, 184, 118}, "NO MEDICAL ID",
                              "Add records before relying on this screen");
  } else {
    AppVisual::drawStatusIcon({12, 34, 37, 37}, AppVisual::StatusIcon::INFO,
                              true);
    WatchyUi::Canvas::centeredText({54, 38, 134, 29},
                                   clipped(profile.name, 18).c_str(), 2,
                                   WatchyUi::Theme::foreground());
    AppVisual::drawDataRow(96, "BLOOD", clipped(profile.bloodType, 18).c_str(),
                           true);
    AppVisual::drawDataRow(116, "ALLERGY", clipped(profile.allergies, 18).c_str());
    AppVisual::drawDataRow(136, "CONDITION", clipped(profile.conditions, 18).c_str());
    AppVisual::drawDataRow(156, "MEDICATION", clipped(profile.medications, 18).c_str());
    AppVisual::drawDataRow(176, "ICE", clipped(profile.icePhone, 18).c_str(),
                           true);
  }
  WatchyUi::Widget::footer("UP/DOWN DETAILS  BACK EXIT");
  WatchyUi::Screen::present();
}
#endif

} // namespace WatchyHealthcareTools
