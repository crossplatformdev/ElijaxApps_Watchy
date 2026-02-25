#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"

void showWatchFace_Kave_Watchy(Watchy &watchy);

namespace WatchfaceRegistryDraw {
namespace {

struct LegacyWatchfaceContext {
  Watchy *watchy;
};

void renderLegacyWatchface(WatchyGxDisplay &, void *userData) {
  auto *ctx = static_cast<LegacyWatchfaceContext *>(userData);
  if (ctx && ctx->watchy) {
    showWatchFace_Kave_Watchy(*ctx->watchy);
  }
}

} // namespace

void drawKaveWatchy(Watchy &watchy) {
  LegacyWatchfaceContext ctx{&watchy};
  UICallbackSpec callbackSpec{renderLegacyWatchface, &ctx};
  UIAppSpec watchfaceSpec{};
  watchfaceSpec.callbacks = &callbackSpec;
  watchfaceSpec.callbackCount = 1;
  UiSDK::renderWatchfaceSpec(watchy, watchfaceSpec);
}

} // namespace WatchfaceRegistryDraw