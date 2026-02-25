#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"

void Watchy::showBuzz() {
  const uint16_t color = UiSDK::getWatchfaceFg(BASE_POLARITY);

  UITextSpec textSpec;
  textSpec.x               = 70;
  textSpec.y               = 80;
  textSpec.w               = 0;
  textSpec.h               = 0;
  textSpec.font            = &FreeMonoBold9pt7b;
  textSpec.fillBackground  = false;
  textSpec.invert          = false;
  textSpec.text            = String("Buzz!");
  UIAppSpec app{};
  app.texts        = &textSpec;
  app.textCount    = 1;
  app.images       = nullptr;
  app.imageCount   = 0;
  app.menus        = nullptr;
  app.menuCount    = 0;
  app.buttons      = nullptr;
  app.buttonCount  = 0;

  UiTemplates::renderBarePage(*this, app);
  vibMotor();
  showMenu(menuIndex);
}
