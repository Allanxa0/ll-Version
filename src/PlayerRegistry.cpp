#include "PlayerRegistry.h"

PlayerRegistry& PlayerRegistry::getInstance() {
    static PlayerRegistry instance;
    return instance;
}

PlayerState& PlayerRegistry::getOrCreate(const string& xuid) {
    return mStates[xuid];
}

PlayerState* PlayerRegistry::get(const string& xuid) {
    auto it = mStates.find(xuid);
    return it != mStates.end() ? &it->second : nullptr;
}

void PlayerRegistry::remove(const string& xuid) {
    mStates.erase(xuid);
}

void PlayerRegistry::clear() {
    mStates.clear();
}
