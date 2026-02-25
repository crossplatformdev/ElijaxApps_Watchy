#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"

void showWatchFace_Maze(Watchy &watchy);

namespace WatchfaceRegistryDraw {
namespace {

struct LegacyWatchfaceContext {
  Watchy *watchy;
};

void renderLegacyWatchface(WatchyGxDisplay &, void *userData) {
  auto *ctx = static_cast<LegacyWatchfaceContext *>(userData);
  if (ctx && ctx->watchy) {
    showWatchFace_Maze(*ctx->watchy);
  }
}

} // namespace

void drawMaze(Watchy &watchy) {
  LegacyWatchfaceContext ctx{&watchy};
  UICallbackSpec callbackSpec{renderLegacyWatchface, &ctx};
  UIAppSpec watchfaceSpec{};
  watchfaceSpec.callbacks = &callbackSpec;
  watchfaceSpec.callbackCount = 1;
  UiSDK::renderWatchfaceSpec(watchy, watchfaceSpec);
}

} // namespace WatchfaceRegistryDraw