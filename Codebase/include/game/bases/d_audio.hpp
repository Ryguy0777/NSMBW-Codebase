#pragma once
#include <game/framework/f_profile.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/snd.h>
#include <nw4r/math.h>

/// @file

/// @ingroup bases

class NMSndObjectBase : nw4r::snd::SoundActor {
    u32 mNumHandles;
    u32 mRemotePlayer;
    void * mSnd2DCalc; // TODO: Snd2DCalc *
    u32 mObjType;
};

template<int T>
class NMSndObject : NMSndObjectBase {
public:
    class SoundHandlePrm : public nw4r::snd::SoundHandle {
    public:
        SoundHandlePrm() : m_04(1.0f) {}

        float m_04;
    };

    virtual void startSound(unsigned long, const nw4r::math::VEC2 &, unsigned long);

    float m_64;
    int m_68;
    float m_6c;
    float m_70;
    SoundHandlePrm mParams[T + 2];
    nw4r::math::VEC2 mPos;
    u32 m_ac;
    u32 m_b0;
    u8 m_b4;
};

class SndObjctPly : public NMSndObject<4> {
public:
    virtual void startSound(unsigned long, const nw4r::math::VEC2 &, unsigned long);
};

class SndObjctCmnEmy : public NMSndObject<4> {
public:
    virtual void startSound(unsigned long, const nw4r::math::VEC2 &, unsigned long);
};

class SndObjctCmnMap : public NMSndObject<4> {
public:
    virtual void startSound(unsigned long, const nw4r::math::VEC2 &, unsigned long);
};

namespace dAudio {
    void requestStartScene(ProfileName sceneProf); ///< Sets up game audio for the given scene.
    void prepareSceneSnd(); ///< @unofficial
    void destroySceneSnd(); ///< @unofficial
    void setNextScene(ProfileName prof, unsigned long bgmIndex);
    void pauseOffGameWithReset(); ///< @unofficial

    int getRemotePlayer(int);
    mVec2_c cvtSndObjctPos(const mVec3_c &);
    bool isForbidRemoteSE();

    bool isBgmAccentSign(unsigned char);

    class SndObjctPlyBase_c : public SndObjctPly {
    };

    class SndObjctPly_c : public SndObjctPlyBase_c {
    public:
        virtual void startSound(ulong, ulong);
    };

    class SndObjctCmnEmy_c : SndObjctCmnEmy {
    public:
        void startSound(unsigned long soundID, const nw4r::math::VEC2 &pos, int remPlayer) {
            SndObjctCmnEmy::startSound(soundID, pos, remPlayer);
        }
        void startSound(unsigned long soundID, const mVec3_c &pos, int remPlayer) {
            SndObjctCmnEmy::startSound(soundID, dAudio::cvtSndObjctPos(pos), remPlayer);
        }
    };

    class SndObjctCmnMap_c : SndObjctCmnMap {
    public:
        void startSound(unsigned long soundID, const nw4r::math::VEC2 &pos, int remPlayer) {
            SndObjctCmnMap::startSound(soundID, pos, remPlayer);
        }
        void startSound(unsigned long soundID, const mVec3_c &pos, int remPlayer) {
            SndObjctCmnMap::startSound(soundID, dAudio::cvtSndObjctPos(pos), remPlayer);
        }
    };

    extern SndObjctCmnEmy_c *g_pSndObjEmy;
    extern SndObjctCmnMap_c *g_pSndObjMap;

    /// @unofficial
    class SoundEffectID_t {
    public:
        SoundEffectID_t(u32 soundID) : id(soundID) {}

        operator u32() const {
            return id;
        }

        template<class T>
        void playObjSound(T *obj, const mVec3_c &pos, int playerNo) const {
            obj->startSound(id, dAudio::cvtSndObjctPos(pos), playerNo);
        }

        void playEmySound(const mVec3_c &pos, int playerNo) const {
            playObjSound(dAudio::g_pSndObjEmy, pos, playerNo);
        }

        void playMapSound(const mVec3_c &pos, int playerNo) const {
            playObjSound(dAudio::g_pSndObjMap, pos, playerNo);
        }

    private:
        u32 id;
    };
};
