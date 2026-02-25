#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"

bool Watchy::connectWiFi()
{
  if (WL_CONNECT_FAILED ==
      WiFi.begin())
  { // WiFi not setup, you can also use hard coded credentials
    // with WiFi.begin(SSID,PASS);
    WIFI_CONFIGURED = false;
  }
  else
  {
    if (WL_CONNECTED ==
        WiFi.waitForConnectResult())
    { // attempt to connect for 10s
      lastIPAddress = WiFi.localIP();
      WiFi.SSID().toCharArray(lastSSID, 30);
      WIFI_CONFIGURED = true;
    }
    else
    { // connection failed, time out
      WIFI_CONFIGURED = false;
      // turn off radios
      WiFi.mode(WIFI_OFF);
      btStop();
    }
  }
  return WIFI_CONFIGURED;
}