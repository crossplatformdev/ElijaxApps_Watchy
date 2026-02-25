#include "WatchyStorage.h"
#include <Preferences.h>

namespace WatchySdk {

bool Storage::read(const char *storageNamespace, const char *key,
                   void *record, size_t recordSize) {
  Preferences preferences;
  if (!preferences.begin(storageNamespace, true)) {
    return false;
  }
  bool validSize = preferences.getBytesLength(key) == recordSize;
  bool read = validSize &&
              preferences.getBytes(key, record, recordSize) == recordSize;
  preferences.end();
  return read;
}

bool Storage::write(const char *storageNamespace, const char *key,
                    const void *record, size_t recordSize) {
  Preferences preferences;
  if (!preferences.begin(storageNamespace, false)) {
    return false;
  }
  bool written = preferences.putBytes(key, record, recordSize) == recordSize;
  preferences.end();
  return written;
}

bool Storage::remove(const char *storageNamespace, const char *key) {
  Preferences preferences;
  if (!preferences.begin(storageNamespace, false)) {
    return false;
  }
  bool removed = !preferences.isKey(key) || preferences.remove(key);
  preferences.end();
  return removed;
}

uint32_t recordChecksum(const void *record, size_t length) {
  const uint8_t *bytes = static_cast<const uint8_t *>(record);
  uint32_t checksum = 2166136261UL;
  for (size_t index = 0; index < length; index++) {
    checksum = (checksum ^ bytes[index]) * 16777619UL;
  }
  return checksum;
}

} // namespace WatchySdk