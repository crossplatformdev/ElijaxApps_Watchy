#include "WatchyUi.h"
#include "Watchy.h"
#include "FallDetection.h"
#include "AppDefaults.h"
#include "HeartRate.h"
#include "SensorManager.h"
#include "WatchyStorage.h"

#include <stddef.h>

namespace FallDetection {
namespace {

constexpr char storageNamespace[] = "watchy-fall";
constexpr char configurationKey[] = "config";
constexpr uint32_t configurationMagic = 0x4c4c4146UL;
constexpr uint32_t traceMagic = 0x45434146UL;
constexpr uint8_t recordVersion = 1;
constexpr uint8_t enabledFlag = 1U << 0;
constexpr uint8_t sampleRateHz = 25;
constexpr uint16_t samplePeriodMs = 1000 / sampleRateHz;
constexpr uint8_t maximumFifoSamples = 170;
constexpr uint8_t preTriggerSamples = 96;
constexpr uint8_t postTriggerSamples = 64;
constexpr uint8_t totalTraceSamples =
    preTriggerSamples + postTriggerSamples;

struct ConfigurationRecord {
  uint32_t magic;
  uint16_t size;
  uint8_t version;
  uint8_t flags;
  uint8_t traceCount;
  uint8_t reserved[3];
  uint32_t nextSequence;
  uint32_t checksum;
};

struct TraceSample {
  int16_t x;
  int16_t y;
  int16_t z;
};

struct TraceRecord {
  uint32_t magic;
  uint16_t size;
  uint8_t version;
  uint8_t sampleRate;
  uint32_t sequence;
  uint32_t capturedAt;
  uint16_t interruptStatus;
  uint8_t preCount;
  uint8_t postCount;
  TraceSample samples[totalTraceSamples];
  uint32_t checksum;
};

struct CachedConfiguration {
  uint32_t magic;
  bool enabled;
  uint8_t traceCount;
  uint32_t nextSequence;
};

static_assert(sizeof(ConfigurationRecord) == 20,
              "Fall configuration layout changed");
static_assert(sizeof(TraceSample) == 6, "Fall sample layout changed");
static_assert(sizeof(TraceRecord) == 984, "Fall trace layout changed");
static_assert(FALL_MONITORING_ANY_MOTION_THRESHOLD <= 0x07ff,
              "Fall any-motion threshold exceeds BMA423 range");
static_assert(FALL_MONITORING_ANY_MOTION_DURATION <= 0x1fff,
              "Fall any-motion duration exceeds BMA423 range");

RTC_DATA_ATTR CachedConfiguration configuration{};
RTC_DATA_ATTR bool sensorArmed = false;

void updateChecksum(ConfigurationRecord &record) {
  record.checksum = WatchySdk::recordChecksum(
      &record, offsetof(ConfigurationRecord, checksum));
}

void updateChecksum(TraceRecord &record) {
  record.checksum = WatchySdk::recordChecksum(
      &record, offsetof(TraceRecord, checksum));
}

bool validConfiguration(const ConfigurationRecord &record) {
  return record.magic == configurationMagic &&
         record.size == sizeof(record) && record.version == recordVersion &&
         !(record.flags & ~enabledFlag) &&
         record.traceCount <= traceCapacity && record.nextSequence > 0 &&
         record.checksum == WatchySdk::recordChecksum(
             &record, offsetof(ConfigurationRecord, checksum));
}

bool validTrace(const TraceRecord &record) {
  return record.magic == traceMagic && record.size == sizeof(record) &&
         record.version == recordVersion &&
         record.sampleRate == sampleRateHz &&
         record.preCount <= preTriggerSamples &&
         record.postCount <= postTriggerSamples &&
         record.checksum == WatchySdk::recordChecksum(
             &record, offsetof(TraceRecord, checksum));
}

void ensureLoaded() {
  if (configuration.magic == configurationMagic) {
    return;
  }
  ConfigurationRecord record{};
  if (WatchySdk::Storage::read(storageNamespace, configurationKey, &record,
                              sizeof(record)) &&
      validConfiguration(record)) {
    configuration = {configurationMagic,
                     (record.flags & enabledFlag) != 0,
                     record.traceCount,
                     record.nextSequence};
    return;
  }
  configuration = {configurationMagic,
                   FALL_MONITORING_DEFAULT_ENABLED != 0, 0, 1};
}

bool saveConfiguration() {
  ConfigurationRecord record{};
  record.magic = configurationMagic;
  record.size = sizeof(record);
  record.version = recordVersion;
  record.flags = configuration.enabled ? enabledFlag : 0;
  record.traceCount = configuration.traceCount;
  record.nextSequence = configuration.nextSequence;
  updateChecksum(record);
  return WatchySdk::Storage::write(storageNamespace, configurationKey,
                                   &record, sizeof(record));
}

bool applySensorConfiguration() {
  sensorArmed = false;
  if (!configuration.enabled || configuration.traceCount >= traceCapacity) {
    return WatchySensor::setBackgroundMode(
        WatchySensor::Mode::Baseline);
  }
  setWatchfaceHeartRateMonitoring(false);
  sensorArmed = WatchySensor::setBackgroundMode(
      WatchySensor::Mode::FallMonitoring);
  return sensorArmed;
}

void copySample(TraceSample &destination, const Accel &source) {
  destination.x = source.x;
  destination.y = source.y;
  destination.z = source.z;
}

void traceKey(uint8_t index, char key[7]) {
  static_assert(traceCapacity <= 10, "Trace keys support one-digit indexes");
  memcpy(key, "trace0", 7);
  key[5] = static_cast<char>('0' + index);
}

bool saveTrace(TraceRecord &trace) {
  if (configuration.traceCount >= traceCapacity) {
    return false;
  }
  updateChecksum(trace);
  uint8_t traceIndex = configuration.traceCount;
  uint32_t previousSequence = configuration.nextSequence;
  char key[7];
  traceKey(traceIndex, key);
  if (!WatchySdk::Storage::write(storageNamespace, key, &trace,
                                 sizeof(trace))) {
    return false;
  }
  configuration.traceCount++;
  configuration.nextSequence++;
  if (saveConfiguration()) {
    return true;
  }
  configuration.traceCount = traceIndex;
  configuration.nextSequence = previousSequence;
  WatchySdk::Storage::remove(storageNamespace, key);
  return false;
}

bool loadTrace(uint8_t index, TraceRecord &trace) {
  if (index >= configuration.traceCount) {
    return false;
  }
  char key[7];
  traceKey(index, key);
  return WatchySdk::Storage::read(storageNamespace, key, &trace,
                                  sizeof(trace)) &&
         validTrace(trace);
}

bool captureTrace(uint16_t interruptStatus) {
  TraceRecord trace{};
  trace.magic = traceMagic;
  trace.size = sizeof(trace);
  trace.version = recordVersion;
  trace.sampleRate = sampleRateHz;
  trace.sequence = configuration.nextSequence;
  tmElements_t currentTime;
  Watchy::RTC.read(currentTime);
  trace.capturedAt = static_cast<uint32_t>(makeTime(currentTime));
  trace.interruptStatus = interruptStatus;

  Accel fifoSamples[maximumFifoSamples];
  uint16_t fifoCount = 0;
  if (WatchySensor::readAccelFifo(fifoSamples, maximumFifoSamples,
                                  fifoCount)) {
    trace.preCount = min<uint16_t>(fifoCount, preTriggerSamples);
    uint16_t first = fifoCount - trace.preCount;
    for (uint8_t index = 0; index < trace.preCount; index++) {
      copySample(trace.samples[index], fifoSamples[first + index]);
    }
  }

  for (uint8_t index = 0; index < postTriggerSamples; index++) {
    Accel acceleration{};
    if (WatchySensor::readAcceleration(acceleration)) {
      copySample(trace.samples[trace.preCount + trace.postCount],
                 acceleration);
      trace.postCount++;
    }
    if (index + 1 < postTriggerSamples) {
      WatchyUi::deepSleepDelay(samplePeriodMs);
    }
  }
  return saveTrace(trace);
}

} // namespace

void initialize() {
  ensureLoaded();
  applySensorConfiguration();
}

Status status() {
  ensureLoaded();
  return Status{configuration.enabled, sensorArmed,
                configuration.traceCount, traceCapacity};
}

bool setEnabled(bool enabled) {
  ensureLoaded();
  bool previous = configuration.enabled;
  configuration.enabled = enabled;
  if (!saveConfiguration()) {
    configuration.enabled = previous;
    return false;
  }
  if (applySensorConfiguration()) {
    return true;
  }
  configuration.enabled = previous;
  saveConfiguration();
  applySensorConfiguration();
  return false;
}

bool clearTraces() {
  ensureLoaded();
  uint8_t previousCount = configuration.traceCount;
  configuration.traceCount = 0;
  if (!saveConfiguration()) {
    configuration.traceCount = previousCount;
    return false;
  }
  bool removed = true;
  for (uint8_t index = 0; index < previousCount; index++) {
    char key[7];
    traceKey(index, key);
    removed &= WatchySdk::Storage::remove(storageNamespace, key);
  }
  return applySensorConfiguration() && removed;
}

bool handleWake() {
  ensureLoaded();
  uint16_t interruptStatus = 0;
  if (!sensorArmed ||
      !WatchySensor::readInterruptStatus(interruptStatus)) {
    return false;
  }
  if (!(interruptStatus & BMA423_ANY_NO_MOTION_INT)) {
    applySensorConfiguration();
    return false;
  }
  bool captured = captureTrace(interruptStatus);
  if (!captured) {
    sensorArmed = false;
    WatchySensor::setBackgroundMode(WatchySensor::Mode::Baseline);
    return false;
  }
  applySensorConfiguration();
  return true;
}

uint64_t wakeMask() {
  ensureLoaded();
  return sensorArmed ? ACC_INT_2_MASK : 0;
}

void exportTraces(Stream &output) {
  ensureLoaded();
  for (uint8_t index = 0; index < configuration.traceCount; index++) {
    TraceRecord trace{};
    if (!loadTrace(index, trace)) {
      output.printf("@WATCHY_FALL_ERROR %u\n", index);
      continue;
    }
    output.printf("@WATCHY_FALL_TRACE 1 %lu %lu %u %u %u %04X\n",
                  static_cast<unsigned long>(trace.sequence),
                  static_cast<unsigned long>(trace.capturedAt),
                  trace.sampleRate, trace.preCount, trace.postCount,
                  trace.interruptStatus);
    uint16_t count = trace.preCount + trace.postCount;
    for (uint16_t sample = 0; sample < count; sample++) {
      bool preTrigger = sample < trace.preCount;
      int32_t relativeMs = preTrigger
          ? -static_cast<int32_t>(trace.preCount - sample) * samplePeriodMs
          : static_cast<int32_t>(sample - trace.preCount) * samplePeriodMs;
      const TraceSample &value = trace.samples[sample];
      output.printf("@WATCHY_FALL_SAMPLE %lu %u %c %ld %d %d %d\n",
                    static_cast<unsigned long>(trace.sequence), sample,
                    preTrigger ? 'P' : 'A', static_cast<long>(relativeMs),
                    value.x, value.y, value.z);
    }
    output.printf("@WATCHY_FALL_END %lu\n",
                  static_cast<unsigned long>(trace.sequence));
  }
  output.printf("@WATCHY_FALL_DONE %u\n", configuration.traceCount);
}

void serviceSerial() {
  static char command[32] = {};
  static uint8_t length = 0;
  while (Serial.available() > 0) {
    int value = Serial.read();
    if (value == '\n') {
      if (length > 0 && command[length - 1] == '\r') {
        length--;
      }
      command[length] = '\0';
      if (strcmp(command, "@WATCHY_FALL_EXPORT 1") == 0) {
        exportTraces(Serial);
      }
      length = 0;
    } else if (length + 1 < sizeof(command)) {
      command[length++] = static_cast<char>(value);
    } else {
      length = 0;
    }
  }
}

} // namespace FallDetection
