#pragma once

#include <string>
#include <unordered_map>

#include "PlayerState.h"

using namespace std;

class PlayerRegistry {
public:
    static PlayerRegistry& getInstance();

    PlayerState& getOrCreate(const string& xuid);
    PlayerState* get(const string& xuid);
    void         remove(const string& xuid);
    void         clear();

    auto& all() { return mStates; }

private:
    unordered_map<string, PlayerState> mStates;
};
