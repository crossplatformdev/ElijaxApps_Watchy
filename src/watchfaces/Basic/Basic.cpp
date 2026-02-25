#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"

void showWatchFace_Basic(Watchy &watchy) {
  char timeBuf[6];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", watchy.currentTime.Hour, watchy.currentTime.Minute);

  UITextSpec title;
  title.x = 5;
  title.y = 18;
  title.font = &FreeMonoBold9pt7b;
  title.fillBackground = false;
  title.invert = false;
  title.text = String("Basic");

  UITextSpec timeText;
  timeText.x = 5;
  timeText.y = 53 + 60;
  timeText.font = &DSEG7_Classic_Bold_53;
  timeText.fillBackground = false;
  timeText.invert = false;
  timeText.text = String(timeBuf);

  UITextSpec texts[2] = {title, timeText};

  UIAppSpec app{};
  app.texts = texts;
  app.textCount = 2;
  app.images = nullptr;
  app.imageCount = 0;
  app.menus = nullptr;
  app.menuCount = 0;
  app.buttons = nullptr;
  app.buttonCount = 0;
  app.checkboxes = nullptr;
  app.checkboxCount = 0;

  UiSDK::renderApp(watchy, app);
}
