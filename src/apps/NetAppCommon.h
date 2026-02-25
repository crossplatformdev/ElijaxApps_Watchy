#pragma once

#include "../watchy/Watchy.h"
#include "../sdk/UiTemplates.h"

namespace NetAppCommon {

UiTemplates::HistoryPickerSpec makePingStyleTargetPicker(const char *nvsNamespace);

bool pickTargetEditAndPersist(Watchy &watchy,
                              const char *nvsNamespace,
                              const char *editorTitle,
                              int8_t &inOutSelectedIndex,
                              char *outTarget,
                              size_t outTargetSize);

} // namespace NetAppCommon
