#include "NetAppCommon.h"

#include "../sdk/UiSDK.h"

namespace NetAppCommon {

UiTemplates::HistoryPickerSpec makePingStyleTargetPicker(const char *nvsNamespace) {
  UiTemplates::HistoryPickerSpec picker;
  picker.title = "Select Target:";
  picker.nvsNamespace = nvsNamespace;
  picker.exampleLabel = "example.com";
  picker.exampleValue = "example.com";
  picker.newLabel = "New address";
  picker.maxEntries = UiTemplates::HISTORY_MAX_ENTRIES;
  picker.maxEntryLen = UiTemplates::HISTORY_MAX_ENTRY_LEN;

  picker.menuLayout.headerX = 16;
  picker.menuLayout.headerY = 36;
  picker.menuLayout.menuX = 0;
  picker.menuLayout.menuY = 72;
  picker.menuLayout.visibleRowsMax = 4;
  picker.menuLayout.startIndex = 0;
  picker.menuLayout.autoScroll = true;
  picker.menuLayout.font = &FreeMonoBold9pt7b;

  return picker;
}

bool pickTargetEditAndPersist(Watchy &watchy,
                              const char *nvsNamespace,
                              const char *editorTitle,
                              int8_t &inOutSelectedIndex,
                              char *outTarget,
                              size_t outTargetSize) {
  if (outTarget == nullptr || outTargetSize == 0) {
    return false;
  }

  UiTemplates::HistoryPickerSpec picker = makePingStyleTargetPicker(nvsNamespace);
  return UiTemplates::runHistoryPickerEditAndPersistNvs(
      watchy, picker, outTarget, outTargetSize, editorTitle, inOutSelectedIndex);
}

} // namespace NetAppCommon
