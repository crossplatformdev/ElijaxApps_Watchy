#ifndef WATCHY_STORAGE_H
#define WATCHY_STORAGE_H

#include <Arduino.h>

namespace WatchySdk {

class Storage {
public:
  static bool read(const char *storageNamespace, const char *key,
                   void *record, size_t recordSize);
  static bool write(const char *storageNamespace, const char *key,
                    const void *record, size_t recordSize);
  static bool remove(const char *storageNamespace, const char *key);
};

uint32_t recordChecksum(const void *record, size_t length);

} // namespace WatchySdk

#endif