#include <kamek.h>
#include <new/bases/d_a_lift_bg_rotation.hpp>
#include <new/bases/d_custom_profile.hpp>
#include <game/bases/d_a_obj_center.hpp>
#include <game/bases/d_a_obj_center2.hpp>

CUSTOM_ACTOR_PROFILE(AC_LIFT_BG_ROTATION, daLiftBgRotation_c, fProfile::EN_SLIP_PENGUIN, fProfile::DRAW_ORDER::EN_SLIP_PENGUIN);

const dActorData_c c_LIFT_BG_ROTATION_actor_data = {fProfile::AC_LIFT_BG_ROTATION, 8, -8, 0, 0, 0, 0, 0, 0, 0, 0, ACTOR_CREATE_GLOBAL};
dCustomProfile_c l_LIFT_BG_ROTATION_profile(&g_profile_AC_LIFT_BG_ROTATION, "AC_LIFT_BG_ROTATION", CourseActor::AC_LIFT_BG_ROTATION, &c_LIFT_BG_ROTATION_actor_data);

bool daLiftBgRotation_c::initMovement() {
    mRotationIdx = (mParam >> 24) & 0xFF;

    mAlwaysUp = (mParam >> 15) & 1;

    dActor_c *controller = (dActor_c *)findController();
    if (controller == nullptr) {
        return false;
    } else {
        mControllerID = controller->mUniqueID;
        
        mDeltaPos.x = mPos.x - controller->mPos.x;
        mDeltaPos.y = mPos.y - controller->mPos.y;

        mOrigin = mVec2_c(mPos.x, mPos.y);
        return true;
    }
};

void daLiftBgRotation_c::executeMovement() {
    dActor_c *controller = (dActor_c *)fManager_c::searchBaseByID(mControllerID);
    if (controller != nullptr) {
        short controllerZ = controller->mAngle.z;
        controllerZ += 0x4000;
        if (!mAlwaysUp) {
            mAngle.y = controllerZ;
        }

        float cosA = nw4r::math::CosF(controllerZ);
        float sinA = nw4r::math::SinF(controllerZ);

        mPos.x = mOrigin.x + (mDeltaPos.x * cosA - mDeltaPos.y * sinA) - mDeltaPos.x;
        mPos.y = mOrigin.y + (mDeltaPos.x * sinA + mDeltaPos.y * cosA) - mDeltaPos.y;
    }
};

fBase_c *daLiftBgRotation_c::findController() {
    fBase_c *controller = nullptr;
    do {
        controller = fManager_c::searchBaseByProfName(fProfile::OBJ_CENTER, controller);
        if (controller == nullptr) {
            controller = nullptr;
            do {
                controller = fManager_c::searchBaseByProfName(fProfile::OBJ_CENTER2, controller);
                if (controller == nullptr) {
                    return nullptr;
                }
            } while (mRotationIdx != ((daObjCenter2_c *)controller)->mRotationIdx);
            return controller;
        }
    } while (mRotationIdx != ((daObjCenter_c *)controller)->mRotationIdx);
    return controller;
}