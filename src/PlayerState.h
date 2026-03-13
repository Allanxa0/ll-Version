#pragma once

#include "mc/legacy/ActorUniqueID.h"

struct PlayerState {
    int           deaths      = 0;
    bool          barEnabled  = true;
    bool          isSneaking  = false;
    bool          isSprinting = false;
    bool          barCreated  = false;
    ActorUniqueID bossID      { 0 };
};
