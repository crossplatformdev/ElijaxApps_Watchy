#include "BcgTraceCapture.h"
#include "WatchyUi.h"
#include "BcgProcessor.h"

#ifdef WATCHY_BCG_TRACE_CAPTURE

namespace WatchyBcgTrace {
namespace {

constexpr uint32_t captureMagic = 0x45474342UL;
constexpr uint16_t captureSeconds = 60;
constexpr uint16_t captureCapacity =
    WatchyBcg::baselineSampleRateMilliHz * captureSeconds / 1000;
constexpr uint16_t slowSampleCapacity = captureCapacity / 2;

struct CaptureState {
  uint32_t magic;
  uint16_t count;
  uint16_t sampleRateMilliHz;
  bool armed;
  bool complete;
  bool autoExport;
};

RTC_DATA_ATTR CaptureState capture{};
RTC_DATA_ATTR WatchyBcg::Sample slowSamples[slowSampleCapacity];
RTC_FAST_ATTR WatchyBcg::Sample
    fastSamples[captureCapacity - slowSampleCapacity];
char command[40] = {};
uint8_t commandLength = 0;

void ensureState() {
  if (capture.magic != captureMagic) {
    capture = {};
    capture.magic = captureMagic;
  }
}

void printStatus() {
  Serial.printf("@WATCHY_BCG_STATUS 1 %u %u %u %u\n",
                capture.count, captureCapacity,
                capture.armed ? 1 : 0, capture.complete ? 1 : 0);
}

WatchyBcg::Sample &sampleAt(uint16_t index) {
  if (index < slowSampleCapacity) return slowSamples[index];
  return fastSamples[index - slowSampleCapacity];
}

void exportTrace() {
  Serial.printf("@WATCHY_BCG_TRACE 1 %u %u\n",
                capture.sampleRateMilliHz, capture.count);
  for (uint16_t index = 0; index < capture.count; index++) {
    const WatchyBcg::Sample &sample = sampleAt(index);
    Serial.printf("@WATCHY_BCG_SAMPLE 1 %u %d %d %d\n", index,
                  sample.x, sample.y, sample.z);
  }
  Serial.printf("@WATCHY_BCG_DONE 1 %u\n", capture.count);
  Serial.flush();
}

void handleCommand() {
  if (strcmp(command, "@WATCHY_BCG_ARM 1") == 0) {
    capture = {};
    capture.magic = captureMagic;
    capture.armed = true;
    Serial.printf("@WATCHY_BCG_ARMED 1 %u\n", captureCapacity);
  } else if (strcmp(command, "@WATCHY_BCG_STATUS 1") == 0) {
    printStatus();
  } else if (strcmp(command, "@WATCHY_BCG_EXPORT 1") == 0) {
    exportTrace();
  } else if (strcmp(command, "@WATCHY_BCG_CLEAR 1") == 0) {
    capture = {};
    capture.magic = captureMagic;
    printStatus();
  }
}

} // namespace

void beginAutomatic(uint16_t sampleRateMilliHz) {
  capture = {};
  capture.magic = captureMagic;
  capture.sampleRateMilliHz = sampleRateMilliHz;
  capture.armed = true;
  capture.autoExport = true;
  Serial.printf("@WATCHY_BCG_ARMED 1 %u\n", captureCapacity);
  Serial.flush();
}

void append(int16_t x, int16_t y, int16_t z,
            uint16_t sampleRateMilliHz) {
  ensureState();
  if (!capture.armed || capture.count >= captureCapacity) return;
  if (capture.count == 0) {
    capture.sampleRateMilliHz = sampleRateMilliHz;
    Serial.printf("@WATCHY_BCG_SAMPLING 1 %u\n", sampleRateMilliHz);
    Serial.flush();
  }
  if (capture.sampleRateMilliHz != sampleRateMilliHz) return;
  sampleAt(capture.count++) = {x, y, z};
  if (capture.count == captureCapacity) {
    capture.armed = false;
    capture.complete = true;
  }
}

bool measurementComplete() {
  ensureState();
  return capture.complete;
}

void serviceSerial() {
  ensureState();
  if (capture.complete && capture.autoExport) {
    capture.autoExport = false;
    exportTrace();
  }
  while (Serial.available() > 0) {
    int value = Serial.read();
    if (value == '\r') continue;
    if (value == '\n') {
      command[commandLength] = '\0';
      handleCommand();
      commandLength = 0;
    } else if (value >= 0x20 && value <= 0x7e &&
               commandLength + 1 < sizeof(command)) {
      command[commandLength++] = static_cast<char>(value);
    } else {
      commandLength = 0;
    }
  }
}

} // namespace WatchyBcgTrace

#endif
