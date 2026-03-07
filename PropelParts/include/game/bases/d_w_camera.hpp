#include <game/mLib/m_sphere.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/mLib/m_mtx.hpp>
#include <lib/egg/gfxe/eggScreen.h>

class dWmViewClip_c {
public:
    void set( float, float, float, float, const mMtx_c& );
    bool intersectSphere(const mSphere_c *); ///< @unofficial
};

class dWCamera_c {
public:
    char pad[0x20c];
    EGG::Screen mScreen;
    char pad2[0x264];
    dWmViewClip_c mViewClip; ///< The camera's view clip.

    static dWCamera_c *m_instance; ///< The instance of the camera.
};
