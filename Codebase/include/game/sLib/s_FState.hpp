#pragma once
#include <game/sLib/s_StateInterfaces.hpp>
#include <game/sLib/s_FStateID.hpp>

/// @brief A state holder for a given class.
/// @tparam T The class that this state belongs to.
/// @ingroup state
template<class T>
class sFState_c : public sStateIf_c {
public:
    sFState_c(T &owner) : mpOwner(owner), mpID(nullptr) {}

    enum STATE_ACTION_e {
        INITIALIZE,
        EXECUTE,
        FINALIZE
    };

    /// @brief Performs a state action.
    /// @details [Gets inlined, needed for function order to match].
    /// @param action The action to perform.
    void performAction(STATE_ACTION_e action) {
        if (action == FINALIZE) {
            mpID->finalizeState(mpOwner);
        } else if (action == EXECUTE) {
            mpID->executeState(mpOwner);
        } else if (action == INITIALIZE) {
            mpID->initializeState(mpOwner);
        }
    }

    virtual void initialize() { performAction(INITIALIZE); }
    virtual void execute() { performAction(EXECUTE); }
    virtual void finalize() { performAction(FINALIZE); }

    void setID(const sFStateID_c<T> *id) { mpID = id; }

private:
    T &mpOwner; ///< The owner of this state.
    const sFStateID_c<T> *mpID; ///< The state ID that runs the state methods.
};
