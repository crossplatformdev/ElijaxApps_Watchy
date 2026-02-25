#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"

void showWatchFace_BinaryBlocks(Watchy &watchy);

namespace WatchfaceRegistryDraw {
namespace {

struct LegacyWatchfaceContext {
  Watchy *watchy;
};

void renderLegacyWatchface(WatchyGxDisplay &, void *userData) {
  auto *ctx = static_cast<LegacyWatchfaceContext *>(userData);
  if (ctx && ctx->watchy) {
    showWatchFace_BinaryBlocks(*ctx->watchy);
  }
}

} // namespace

void drawBinaryBlocks(Watchy &watchy) {
  LegacyWatchfaceContext ctx{&watchy};
  UICallbackSpec callbackSpec{renderLegacyWatchface, &ctx};
  UIAppSpec watchfaceSpec{};
  watchfaceSpec.callbacks = &callbackSpec;
  watchfaceSpec.callbackCount = 1;
  UiSDK::renderWatchfaceSpec(watchy, watchfaceSpec);
}

} // namespace WatchfaceRegistryDraw