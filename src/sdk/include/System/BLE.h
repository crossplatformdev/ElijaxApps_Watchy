#ifndef _BLE_H_
#define _BLE_H_

#include "Arduino.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef BLE_42_FEATURE_SUPPORT
#define BLE_42_FEATURE_SUPPORT 1
#endif
#ifndef BLE_50_FEATURE_SUPPORT
#define BLE_50_FEATURE_SUPPORT 1
#endif

#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "esp_ota_ops.h"

#include "config.h"

class BLE;

class BLE {
public:
  BLE(void);
  ~BLE(void);

  bool begin(const char *localName);
  int updateStatus();
  int howManyBytes();

private:
  String local_name;

  BLEServer *pServer = NULL;

  BLEService *pESPOTAService                 = NULL;
  BLECharacteristic *pESPOTAIdCharacteristic = NULL;

  BLEService *pService                            = NULL;
  BLECharacteristic *pVersionCharacteristic       = NULL;
  BLECharacteristic *pOtaCharacteristic           = NULL;
  BLECharacteristic *pWatchFaceNameCharacteristic = NULL;
};

#endif
