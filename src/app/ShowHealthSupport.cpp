#include "WatchyUi.h"
#include "Watchy.h"
#include <Preferences.h>
#include <stddef.h>
#include "AppDisplay.h"
#include "MetronomeEngine.h"
#include "NetworkAppCommon.h"
#include "WatchyStorage.h"

namespace {

enum HealthSupportTool : uint8_t {
  BREATHING_COACH,
  CPR_METRONOME,
  RECOVERY_POSITION,
  STROKE_FAST,
  CHOKING_RESPONSE,
  SEIZURE_AID,
  SEVERE_BLEEDING,
  BURN_FIRST_AID,
  HEAT_EMERGENCY,
  HYPOTHERMIA,
  POISONING,
  ANAPHYLAXIS,
  OPIOID_OVERDOSE,
  ASTHMA_ATTACK,
  EMERGENCY_NUMBERS,
  PAIN_LOG,
  SYMPTOM_NOTE,
  GROUNDING,
  HEALTH_SUPPORT_TOOL_COUNT
};

struct Guide {
  const char *title;
  const char *lines[10];
  uint8_t lineCount;
};

const Guide guides[] = {
    {"RECOVERY POSITION",
     {"Unresponsive but breathing:", "1 Call emergency services", "2 Place on side if safe",
      "3 Keep airway open", "4 Monitor normal breathing", "Suspected spine injury:",
      "avoid needless movement", "Follow dispatcher guidance"}, 8},
    {"STROKE - FAST",
     {"F  Face drooping", "A  Arm weakness", "S  Speech difficulty", "T  Time to call emergency",
      "Note symptom start time", "Do not give food or drink", "Act even if symptoms stop"}, 7},
    {"CHOKING RESPONSE",
     {"Can cough or speak:", "encourage coughing", "Cannot breathe or speak:",
      "call emergency services", "Give age-appropriate first aid", "only if trained",
      "If unconscious: CPR + AED", "Follow dispatcher guidance"}, 8},
    {"SEIZURE FIRST AID",
     {"Protect from nearby hazards", "Cushion head; loosen neckwear", "Do NOT restrain", "Nothing in the mouth",
      "Time the seizure", "After: side position, monitor", "Call if >5 min, injured,", "pregnant, water, or first"}, 8},
    {"SEVERE BLEEDING",
     {"Call emergency services", "Use gloves/barrier if possible", "Apply firm direct pressure", "Keep steady pressure",
      "Add dressings; do not remove", "Use tourniquet only if", "trained / dispatcher directs", "Treat for shock; keep warm"}, 8},
    {"BURN FIRST AID",
     {"Stop the burning source", "Cool under running water", "for 20 minutes", "Remove jewelry if not stuck",
      "Cover loosely, keep warm", "No ice, creams, or grease", "Call for large/deep/electrical", "chemical/airway burns"}, 8},
    {"HEAT EMERGENCY",
     {"Confusion, collapse, hot skin", "can mean heat stroke", "CALL EMERGENCY SERVICES", "Move to cooler place",
      "Cool rapidly with water,", "wet cloths, fans, ice packs", "Do not force drink if confused", "Monitor breathing"}, 8},
    {"HYPOTHERMIA",
     {"Call emergency services", "Move gently to shelter", "Remove wet clothes", "Warm the body core slowly",
      "Use dry layers / blankets", "No alcohol or direct hot bath", "Monitor breathing", "Start CPR if directed"}, 8},
    {"POISONING",
     {"Call poison center / EMS", "Keep product/container", "Move from fumes if safe", "Rinse exposed skin/eyes",
      "Do NOT induce vomiting", "unless professionals instruct", "Do not give food or drink", "Follow expert directions"}, 8},
    {"ANAPHYLAXIS",
     {"Breathing trouble / swelling", "after allergen = emergency", "Use prescribed epinephrine", "immediately as trained",
      "Call emergency services", "Lie safely; avoid standing", "Second dose only per plan", "Monitor breathing / CPR"}, 8},
    {"OPIOID OVERDOSE",
     {"Unresponsive + slow/no breath", "Call emergency services", "Give naloxone if available", "and trained to use it",
      "Rescue breathing / CPR", "as trained or dispatched", "Use AED if available", "Stay; repeat naloxone per label"}, 8},
    {"ASTHMA ATTACK",
     {"Sit upright; stay calm", "Use prescribed reliever", "per personal action plan", "Call emergency services if",
      "severe, worsening, blue lips,", "cannot speak, or no relief", "Do not lie flat", "Follow dispatcher guidance"}, 8},
    {"EMERGENCY NUMBERS",
     {"EU / GSM emergency: 112", "US / Canada: 911", "UK / Ireland: 999 or 112", "Use your local emergency no.",
      "Poison center varies by country", "WATCHY CANNOT PLACE CALLS", "Give location and condition", "Follow dispatcher guidance"}, 8}};

static_assert(sizeof(guides) / sizeof(guides[0]) ==
                  EMERGENCY_NUMBERS - RECOVERY_POSITION + 1,
              "First-aid guide registry is incomplete");

constexpr const char *healthNamespace = "watchy-health";
constexpr const char *painRecordKey = "painLog";
constexpr uint32_t painRecordMagic = 0x4e494150UL;
constexpr uint8_t painRecordVersion = 1;

struct PainRecord {
  uint32_t magic;
  uint16_t size;
  uint8_t version;
  uint8_t pain;
  uint32_t timestamp;
  uint32_t checksum;
};

static_assert(sizeof(PainRecord) == 16, "Pain record layout changed");

void updatePainChecksum(PainRecord &record) {
  record.checksum = WatchySdk::recordChecksum(
      &record, offsetof(PainRecord, checksum));
}

bool validPainRecord(const PainRecord &record) {
  return record.magic == painRecordMagic && record.size == sizeof(record) &&
         record.version == painRecordVersion && record.pain <= 10 &&
         record.checksum == WatchySdk::recordChecksum(
             &record, offsetof(PainRecord, checksum));
}

PainRecord loadPainRecord() {
  PainRecord record{};
  if (WatchySdk::Storage::read(healthNamespace, painRecordKey,
                              &record, sizeof(record)) &&
      validPainRecord(record)) {
    return record;
  }
  Preferences preferences;
  record.magic = painRecordMagic;
  record.size = sizeof(record);
  record.version = painRecordVersion;
  if (preferences.begin(healthNamespace, true)) {
    record.pain = min<uint8_t>(preferences.getUChar("pain", 0), 10);
    record.timestamp = preferences.getUInt("painAt", 0);
    preferences.end();
  }
  updatePainChecksum(record);
  WatchySdk::Storage::write(healthNamespace, painRecordKey,
                            &record, sizeof(record));
  return record;
}

bool savePainRecord(uint8_t pain, uint32_t timestamp) {
  PainRecord record{};
  record.magic = painRecordMagic;
  record.size = sizeof(record);
  record.version = painRecordVersion;
  record.pain = min<uint8_t>(pain, 10);
  record.timestamp = timestamp;
  updatePainChecksum(record);
  return WatchySdk::Storage::write(healthNamespace, painRecordKey,
                                  &record, sizeof(record));
}

void sanitizeNote(String &note, size_t maximumLength) {
  if (note.length() > maximumLength) note.remove(maximumLength);
  for (size_t index = 0; index < note.length(); index++) {
    uint8_t character = static_cast<uint8_t>(note[index]);
    if (character < 32 || character == 127) note.setCharAt(index, ' ');
  }
  note.trim();
  if (note.length() == 0) note = "NOT SET";
}

void pulseMotor(uint16_t durationMs) {
  Watchy::vibMotor(durationMs, 1);
}

String guideText(const Guide &guide) {
  String text;
  for (uint8_t line = 0; line < guide.lineCount; line++) {
    if (line != 0) text += '\n';
    text += String(line + 1);
    text += ". ";
    text += guide.lines[line];
  }
  return text;
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void drawGuide(const Guide &guide) {
  String text = guideText(guide);
  WatchyUi::ScrollableTextView::draw(WatchyUi::ScrollableTextModel{
      guide.title, text.c_str(), "UP/DOWN SCROLL     BACK EXIT",
      0, 31, 16, 9});
  WatchyUi::Screen::present();
}
#endif

void runGuide(const Guide &guide) {
  String text = guideText(guide);
  WatchyUi::ScrollableTextView::show(guide.title, text.c_str());
}

void drawBreathing(const char *phase, uint8_t cycle, bool running) {
  char status[20];
  snprintf(status, sizeof(status), "CYCLE %u", cycle);
  float progress = !strcmp(phase, "INHALE") ? 0.25f
                   : !strcmp(phase, "HOLD") && cycle % 2 ? 0.50f
                   : !strcmp(phase, "EXHALE") ? 0.75f : 1.0f;
  WatchyUi::ValueModel model{
      "BREATHING COACH", phase, status,
      "Box breathing 4-4-4-4. Stop if dizzy.",
      running ? "SELECT PAUSE       BACK EXIT"
              : "SELECT RESUME      BACK EXIT", progress};
  WatchyUi::ValueView::draw(model);
  WatchyUi::Screen::present();
}

void runBreathing() {
  WatchyUi::Input::begin();
  const char *const phases[] = {"INHALE", "HOLD", "EXHALE", "HOLD"};
  uint8_t phase = 0;
  uint8_t cycle = 1;
  bool running = true;
  uint32_t nextPhase = millis() + 4000;
  drawBreathing(phases[phase], cycle, running);
  pulseMotor(40);
  while (true) {
    uint32_t waitMs = UINT32_MAX;
    if (running) {
      int32_t remaining = static_cast<int32_t>(nextPhase - millis());
      waitMs = remaining > 0 ? static_cast<uint32_t>(remaining) : 0;
    }
    WatchyUi::Event event = WatchyUi::Input::wait(waitMs);
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    if (event == WatchyUi::Event::MENU) {
      running = !running;
      nextPhase = millis() + 4000;
      drawBreathing(phases[phase], cycle, running);
    }
    if (running && static_cast<int32_t>(millis() - nextPhase) >= 0) {
      phase = (phase + 1) % 4;
      if (phase == 0) cycle++;
      nextPhase += 4000;
      pulseMotor(40);
      drawBreathing(phases[phase], cycle, running);
    }
  }
}

void drawCpr(bool running) {
  WatchyUi::ValueModel model{
      "CPR METRONOME", "110", running ? "RUNNING" : "READY",
      "Call EMS. Get AED. Follow dispatcher and training.",
      running ? "SELECT PAUSE       BACK EXIT"
              : "SELECT START       BACK EXIT", 110.0f / 120.0f};
  WatchyUi::ValueView::draw(model);
  WatchyUi::Screen::present();
}

void runCprMetronome() {
  WatchyUi::Input::begin();
  bool running = false;
  drawCpr(running);
  while (true) {
    WatchyUi::Event event = running
                                ? WatchyUi::Input::waitScheduled()
                                : WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      if (running) {
        WatchyMetronome::stop();
      }
      return;
    }
    if (event == WatchyUi::Event::MENU) {
      if (running) {
        WatchyMetronome::stop();
        running = false;
      } else {
        running = WatchyMetronome::start(
            110, 1, WatchyMetronome::PulseStyle::Uniform);
        if (!running) {
          WatchyUi::Feedback::showMessage(
              "CPR METRONOME", "Unable to start the timing worker.",
              WatchyUi::MessageKind::ERROR, "BACK");
          WatchyUi::Input::wait();
        }
      }
      drawCpr(running);
    }
  }
}

void drawPain(uint8_t pain, uint8_t lastPain, uint32_t lastTime) {
  char value[4];
  char status[16];
  char detail[48];
  snprintf(value, sizeof(value), "%u", pain);
  snprintf(status, sizeof(status), "LAST %u / 10", lastPain);
  snprintf(detail, sizeof(detail), "Saved epoch: %lu. Self-report only.",
           static_cast<unsigned long>(lastTime));
  WatchyUi::ValueModel model{"PAIN LOG", value, status, detail,
                             "UP/DOWN SCORE SELECT SAVE BACK", pain / 10.0f};
  WatchyUi::ValueView::draw(model);
  WatchyUi::Screen::present();
}

void runPainLog(Watchy *watchy) {
  WatchyUi::Input::begin();
  PainRecord record = loadPainRecord();
  uint8_t lastPain = record.pain;
  uint32_t lastTime = record.timestamp;
  uint8_t pain = lastPain;
  drawPain(pain, lastPain, lastTime);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    if (event == WatchyUi::Event::UP) {
      pain = WatchyUi::Selector::step(pain, 1, 0, 10);
      drawPain(pain, lastPain, lastTime);
    } else if (event == WatchyUi::Event::DOWN) {
      pain = WatchyUi::Selector::step(pain, -1, 0, 10);
      drawPain(pain, lastPain, lastTime);
    } else if (event == WatchyUi::Event::MENU) {
      tmElements_t currentTime;
      if (watchy != nullptr) {
        watchy->RTC.read(watchy->currentTime);
        currentTime = watchy->currentTime;
      } else {
        WatchySdk::RTC.read(WatchySdk::currentTime);
        currentTime = WatchySdk::currentTime;
      }
      uint32_t candidateTime = static_cast<uint32_t>(makeTime(currentTime));
      bool saved = savePainRecord(pain, candidateTime);
      if (saved) {
        lastPain = pain;
        lastTime = candidateTime;
        pulseMotor(60);
      }
      drawPain(pain, lastPain, lastTime);
      WatchyUi::Feedback::toast(saved ? "PAIN LOG SAVED"
                                       : "SETTING NOT SAVED");
    }
  }
}

void runSymptomNote() {
  Preferences preferences;
  String current = HEALTHCARE_SYMPTOM_NOTE_TEXT;
  if (preferences.begin("watchy-health", true)) {
    current = preferences.getString("symptom", HEALTHCARE_SYMPTOM_NOTE_TEXT);
    preferences.end();
  }
  sanitizeNote(current, 80);
  String edited;
  if (NetworkApps::editText("SYMPTOM NOTE", current, edited, 80)) {
    bool saved = false;
    if (preferences.begin("watchy-health", false)) {
      saved = preferences.putString("symptom", edited) > 0;
      preferences.end();
    }
    if (saved) current = edited;
  }
  WatchyUi::Feedback::showMessage(
      "SYMPTOM NOTE", current.c_str(), WatchyUi::MessageKind::INFO,
      "LOCAL NOTE - NOT TRANSMITTED");
}

void drawGrounding(uint8_t step) {
  const char *const counts[] = {"5", "4", "3", "2", "1"};
  const char *const prompts[] = {"THINGS YOU CAN SEE", "THINGS YOU CAN FEEL",
                                 "THINGS YOU CAN HEAR", "THINGS YOU CAN SMELL",
                                 "THING YOU CAN TASTE"};
  WatchyUi::ValueModel model{
      "GROUNDING 5-4-3-2-1", counts[step], prompts[step],
      "Notice slowly. Breathe normally.", "SELECT NEXT        BACK EXIT",
      static_cast<float>(step + 1) / 5.0f};
  WatchyUi::ValueView::draw(model);
  WatchyUi::Screen::present();
}

void runGrounding() {
  WatchyUi::Input::begin();
  uint8_t step = 0;
  drawGrounding(step);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    if (event == WatchyUi::Event::MENU) {
      step = WatchyUi::ListView::next(step, 5);
      pulseMotor(40);
      drawGrounding(step);
    }
  }
}

} // namespace

void showHealthSupportToolImpl(uint8_t tool, Watchy *watchy) {
  if (tool == BREATHING_COACH) {
    runBreathing();
    if (watchy != nullptr) watchy->showMenu(menuIndex, false);
    else WatchySdk::showMenu(menuIndex, false);
    return;
  }
  if (tool == CPR_METRONOME) {
    runCprMetronome();
    if (watchy != nullptr) watchy->showMenu(menuIndex, false);
    else WatchySdk::showMenu(menuIndex, false);
    return;
  }
  if (tool >= RECOVERY_POSITION && tool <= EMERGENCY_NUMBERS) {
    runGuide(guides[tool - RECOVERY_POSITION]);
    if (watchy != nullptr) watchy->showMenu(menuIndex, false);
    else WatchySdk::showMenu(menuIndex, false);
    return;
  }
  if (tool == PAIN_LOG) {
    runPainLog(watchy);
    if (watchy != nullptr) watchy->showMenu(menuIndex, false);
    else WatchySdk::showMenu(menuIndex, false);
    return;
  }
  if (tool == SYMPTOM_NOTE) {
    runSymptomNote();
    return;
  }
  if (tool == GROUNDING) {
    runGrounding();
    if (watchy != nullptr) watchy->showMenu(menuIndex, false);
    else WatchySdk::showMenu(menuIndex, false);
  }
}

void Watchy::showHealthSupportTool(uint8_t tool) {
  showHealthSupportToolImpl(tool, this);
}

void WatchySdk::showHealthSupportTool(uint8_t tool) {
  showHealthSupportToolImpl(tool, nullptr);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderHealthSupportPreview(uint8_t tool, uint8_t view) {
  if (tool == BREATHING_COACH) {
    const char *const phases[] = {"INHALE", "HOLD", "EXHALE", "HOLD"};
    drawBreathing(phases[min<uint8_t>(view, 3)], 2, view != 3);
  } else if (tool == CPR_METRONOME) {
    drawCpr(view != 0);
  } else if (tool >= RECOVERY_POSITION && tool <= EMERGENCY_NUMBERS) {
    drawGuide(guides[tool - RECOVERY_POSITION]);
  } else if (tool == PAIN_LOG) {
    drawPain(view == 0 ? 0 : 4, view == 2 ? 4 : 0,
             view == 2 ? 1787473800UL : 0);
  } else if (tool == SYMPTOM_NOTE) {
    if (view == 2) {
      NetworkApps::renderTextEditorPreview(
          "SYMPTOM NOTE", "Mild headache since 09:30.");
    } else {
      WatchyUi::Feedback::showMessage(
          "SYMPTOM NOTE", view == 0 ? "Mild headache since 09:30." : "NOT SET",
          WatchyUi::MessageKind::INFO, "LOCAL NOTE - NOT TRANSMITTED");
    }
  } else if (tool == GROUNDING) {
    drawGrounding(min<uint8_t>(view, 4));
  }
}

} // namespace WatchyDemo
#endif
