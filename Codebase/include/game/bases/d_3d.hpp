#pragma once
#include <game/mLib/m_3d.hpp>

namespace d3d {

bool createLightMgr(EGG::Heap *heap, unsigned short lightCount, unsigned short ambLightCount, unsigned char, bool makeFog, int sceneID);

class proc_c : public m3d::proc_c {

};

} // namespace d3d