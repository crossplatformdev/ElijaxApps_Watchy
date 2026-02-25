#include "ScreenCapture.h"

#ifdef WATCHY_DETERMINISTIC_GALLERY

#include <cstring>

namespace WatchyDemo {
namespace ScreenCapture {
namespace {

constexpr uint32_t READY_INTERVAL_MS = 1000;
constexpr size_t MAX_SCENE_ID_LENGTH = 95;
constexpr size_t MAX_COMMAND_LENGTH = 31;

char sceneId[MAX_SCENE_ID_LENGTH + 1] = {};
uint32_t sequence = 0;
bool armed = false;

uint32_t crc32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xFFFFFFFFUL;
  for (size_t index = 0; index < length; index++) {
    crc ^= data[index];
    for (uint8_t bit = 0; bit < 8; bit++) {
      crc = (crc >> 1) ^ (0xEDB88320UL &
                          static_cast<uint32_t>(-(crc & 1UL)));
    }
  }
  return ~crc;
}

} // namespace

void begin() {
  Serial.begin(115200);
  char command[MAX_COMMAND_LENGTH + 1] = {};
  size_t commandLength = 0;
  uint32_t lastReadyAt = millis() - READY_INTERVAL_MS;
  while (true) {
    uint32_t now = millis();
    if (now - lastReadyAt >= READY_INTERVAL_MS) {
      Serial.println("@WATCHY_READY 1");
      Serial.flush();
      lastReadyAt = now;
    }
    while (Serial.available() > 0) {
      int value = Serial.read();
      if (value == '\n') {
        if (commandLength > 0 && command[commandLength - 1] == '\r') {
          commandLength--;
        }
        command[commandLength] = '\0';
        if (strcmp(command, "@WATCHY_CAPTURE 1") == 0) {
          sequence = 0;
          armed = false;
          sceneId[0] = '\0';
          Serial.println("@WATCHY_GALLERY 1");
          Serial.flush();
          return;
        }
        commandLength = 0;
      } else if (commandLength < MAX_COMMAND_LENGTH) {
        command[commandLength++] = static_cast<char>(value);
      } else {
        commandLength = 0;
      }
    }
    delay(10);
  }
}

void arm(const char *nextSceneId) {
  if (armed) {
    error("scene-not-emitted", sequence + 1, sequence);
  }
  if (nextSceneId == nullptr || nextSceneId[0] == '\0') {
    nextSceneId = "unlabeled";
  }
  strncpy(sceneId, nextSceneId, MAX_SCENE_ID_LENGTH);
  sceneId[MAX_SCENE_ID_LENGTH] = '\0';
  armed = true;
}

void emit(const uint8_t *bitmap, uint16_t width, uint16_t height) {
  if (!armed) {
    error("unexpected-frame", sequence + 1, sequence);
    return;
  }
  if (bitmap == nullptr || width == 0 || height == 0) {
    error("invalid-frame", sequence + 1, sequence);
    return;
  }

  armed = false;
  sequence++;
  size_t byteCount =
      (static_cast<size_t>(width) * static_cast<size_t>(height) + 7U) / 8U;
  uint32_t checksum = crc32(bitmap, byteCount);
  Serial.printf("@WATCHY_FRAME 1 %lu %u %u %u %08lX %s\n",
                static_cast<unsigned long>(sequence), width, height,
                static_cast<unsigned int>(byteCount),
                static_cast<unsigned long>(checksum), sceneId);
  Serial.write(bitmap, byteCount);
  Serial.printf("\n@WATCHY_END %lu\n",
                static_cast<unsigned long>(sequence));
  Serial.flush();
}

void finish(uint16_t expectedFrames) {
  if (armed) {
    error("scene-not-emitted", expectedFrames, sequence);
  }
  if (sequence != expectedFrames) {
    error("frame-count", expectedFrames, sequence);
  }
  Serial.printf("@WATCHY_DONE %lu %u\n",
                static_cast<unsigned long>(sequence), expectedFrames);
  Serial.flush();
}

void error(const char *message, uint16_t expected, uint16_t actual) {
  Serial.printf("@WATCHY_ERROR %s expected=%u actual=%u\n", message,
                expected, actual);
  Serial.flush();
}

} // namespace ScreenCapture
} // namespace WatchyDemo

#endif