#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_s_koopatlas.hpp>
#include <game/bases/d_player_model.hpp>

class daKPPlayer_c : public dBaseActor_c {
public:
    int create();
    int doDelete();
    int execute();
    int draw();

    void startAnimation(int id, float frame, float unk, float updateRate);
    void bindPats();

    void setTargetRotY(s16 value) {
        mTargetRotY = value;
        mIsSpinning = false;
    }

    void setSpinning() {
        mIsSpinning = true;
    }

    mHeapAllocator_c mAllocator;
    m3d::anmTexPat_c mTexPats[4];
    mEf::levelEffect_c mEffect;
    dPyMdlMng_c *mpPyMdlMng;

    int mCurrentAnim;
    int mSoundName;
    int mStepSndTimer;

    s16 mTargetRotY;

    float mCurrentFrame;
    float mCurrentUnk;
    float mCurrentUpdateRate;
    float mJumpOffset;

    const char *mpEffectName;

    bool mHasEffect;
    bool mHasSound;
    bool mStep;
    bool mIsSpinning;

    static daKPPlayer_c *m_instance;
};
#endif