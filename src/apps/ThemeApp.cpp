#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"

void Watchy::showInvertColors() {
  guiState = APP_STATE;
  UiTemplates::waitForAllButtonsReleased();
  // Toggle global dark/light theme flag
  gDarkMode = !gDarkMode;

  UITextSpec text{};
  text.x               = 0;
  text.y               = 40;
  text.w               = 0;
  text.h               = 0;
  text.font            = &FreeMonoBold9pt7b;
  text.fillBackground  = false;
  text.invert          = false;
  text.text            = gDarkMode
                         ? String("Theme: DARK\nBG: BLACK\nFG: WHITE")
                         : String("Theme: LIGHT\nBG: WHITE\nFG: BLACK");

  UIAppSpec app{};
  app.texts        = &text;
  app.textCount    = 1;
  app.images       = nullptr;
  app.imageCount   = 0;
  app.menus        = nullptr;
  app.menuCount    = 0;
  app.buttons      = nullptr;
  app.buttonCount  = 0;
  app.checkboxes   = nullptr;
  app.checkboxCount= 0;
  app.scrollTexts  = nullptr;
  app.scrollTextCount = 0;
  app.callbacks    = nullptr;
  app.callbackCount= 0;

  UiSDK::renderApp(*this, app);
  delay(100);
  showMenu(menuIndex);
}
