#pragma once

#include <lib/egg/gfxe/eggModelEx.h>

namespace EGG {

class LightTextureManager {
public:
    void drawAndCaptureTexture(float, float, float, float);
    int getTextureIndex(char const *) const;
    bool replaceModelTexture(int, EGG::ModelEx *) const;

    void LoadBinary(void *); ///< @unofficial
};

} // namespace EGG
