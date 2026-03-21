#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>

class dBossLife_Common_c {
public:
    virtual ~dBossLife_Common_c();

    virtual bool isNonDamage() const;
    virtual bool isOneDamage() const;
    virtual bool isTwoDamage() const;
    virtual bool isDmgSection() const;

    virtual int getDamage_Fire() const;
    virtual int getDamage_Fumi() const;
    virtual int getDamage_HipAtk() const;
    virtual int getDamage_Star() const;
    virtual int getDamage_PenguinSlide() const;
    virtual int getDamage_BlockHit() const;
    virtual int getDamage_Shell() const;
    virtual int getDamage_Quake() const;

    virtual void damageRev(int);

    int mHealth;
};

class dBossLifeInf_c : public dBossLife_Common_c {
public:
    virtual ~dBossLifeInf_c();

    virtual bool isNonDamage() const = 0;
    virtual bool isOneDamage() const = 0;
    virtual bool isTwoDamage() const = 0;
    virtual bool isDmgSection() const;

    virtual int getDamage_Fire() const = 0;
    virtual int getDamage_Fumi() const = 0;
    virtual int getDamage_HipAtk() const = 0;
    virtual int getDamage_Star() const = 0;
    virtual int getDamage_PenguinSlide() const = 0;
    virtual int getDamage_BlockHit() const = 0;
    virtual int getDamage_Shell() const = 0;
    virtual int getDamage_Quake() const = 0;

    virtual void damageRev(int);
};

class dEnBoss_c : public dEn_c {
public:
    dEnBoss_c();
    virtual ~dEnBoss_c();

    virtual int create();

    virtual void Normal_VsPlHitCheck(dCc_c *self, dCc_c *other);
    virtual void Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other);

    virtual bool hitCallback_Star(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Slip(dCc_c *self, dCc_c *other);

    virtual bool hitCallback_Spin(dCc_c *self, dCc_c *other);

    virtual bool hitCallback_WireNet(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_HipAttk(dCc_c *self, dCc_c *other);

    virtual bool hitCallback_PenguinSlide(dCc_c *self, dCc_c *other);

    virtual bool hitCallback_Shell(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Fire(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Ice(dCc_c *self, dCc_c *other);

    virtual void setDeathInfo_Quake(int);

    virtual bool isQuakeDamage();

    STATE_VIRTUAL_FUNC_DECLARE(dEnBoss_c, DieFumi);

    virtual void FumiScoreSet(dActor_c *actor);

    STATE_VIRTUAL_FUNC_DECLARE(dEnBoss_c, DemoWait);
    STATE_VIRTUAL_FUNC_DECLARE(dEnBoss_c, DieFire);
    STATE_VIRTUAL_FUNC_DECLARE(dEnBoss_c, DieSlide);
    STATE_VIRTUAL_FUNC_DECLARE(dEnBoss_c, DieShell);
    STATE_VIRTUAL_FUNC_DECLARE(dEnBoss_c, DieStar);
    STATE_VIRTUAL_FUNC_DECLARE(dEnBoss_c, DieQuake);

    virtual void setBattleReady();
    virtual void createModel();
    virtual void createBossLife();
    virtual int createInit();

    virtual void tenmetsuReady();
    virtual void tenmetsuProc();
    virtual void tenmetsuFin();

    virtual int getTenmetsuTime_Fire();
    virtual int getTenmetsuTime_Shell();
    virtual int getTenmetsuTime_Press();

    virtual void deadAllKill();

    virtual void setFumiDamage(dActor_c *);
    virtual void setFumiDead(dActor_c *);
    virtual void setFireDamage(dActor_c *);
    virtual void setFireDead(dActor_c *);
    virtual void setHipatkDamage(dActor_c *);
    virtual void setHipatkDead(dActor_c *);
    virtual void setSlideDamage(dActor_c *);
    virtual void setSlideDead(dActor_c *);
    virtual void setStarDamage(dActor_c *);
    virtual void setStarDead(dActor_c *);
    virtual void setQuakeDamage();
    virtual void setQuakeDead();
    virtual void setShellDamage(dActor_c *);
    virtual void setShellDead(dActor_c *);

    virtual void damageProc();
    virtual void deadProc();

    virtual bool isFumiInvalid() const;
    virtual bool isFumiDmgInvalid() const;
    virtual bool isFireInvalid() const;
    virtual bool isSlideInvalid() const;
    virtual bool isShellInvalid() const;
    virtual bool isStarInvalid() const;

    virtual void fumideadEffect();
    virtual void fumidmgEffect();
    virtual void hitFireEffect();
    virtual void hitShellEffect();

    virtual void fumidmgSE();
    virtual void fumideadSE();
    virtual void stardmgSE();
    virtual void stardeadSE();
    virtual void shelldmgSE();
    virtual void shelldeadSE();
    virtual void firedmgSE();
    virtual void firedeadSE();
    virtual void quakedmgSE();
    virtual void quakedeadSE();

    virtual void fumiDeadVo();
    virtual void damageSVo();
    virtual void damageLVo();

    dHeapAllocator_c mAllocator;
    int mTenmetsuTime;
    u8 mSound[0xAC]; // dAudio::SndObjctEmy_c
    u16 mSoundParam;
    u32 mCanQuake;
    dBossLife_Common_c *mpBossLife;
    u32 mPad;
    nw4r::g3d::ResFile mRes;
    m3d::mdl_c mModel;
    m3d::anmChr_c mAnmChr;
    m3d::anmMatClr_c mAnmClr;
};