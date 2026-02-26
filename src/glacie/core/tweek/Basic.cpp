#include "ll/api/memory/Hook.h"
#include "mc/world/level/Level.h"

LL_AUTO_TYPE_INSTANCE_HOOK(
    BlockNetworkIdsAreHashesHook,
    ll::memory::HookPriority::Normal,
    Level,
    "?blockNetworkIdsAreHashes@Level@@UEBA_NXZ",
    bool
) {
    return true;
}
