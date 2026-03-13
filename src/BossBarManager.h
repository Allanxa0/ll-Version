#pragma once

#include <string>

#include "Config.h"
#include "PlayerState.h"

#include "mc/server/ServerPlayer.h"
#include "mc/world/level/Level.h"
#include "mc/world/actor/ai/util/BossBarColor.h"
#include "mc/world/actor/ai/util/BossBarOverlay.h"
#include "mc/world/actor/ai/util/BossEventUpdateType.h"

using namespace std;

class BossBarManager {
public:
    static BossBarManager& getInstance();

    void setConfig(const PluginConfig* config);

    void create(ServerPlayer& player, PlayerState& state);
    void remove(ServerPlayer& player, PlayerState& state);
    void update(ServerPlayer& player, PlayerState& state, Level& level);

private:
    void send(
        ServerPlayer&       player,
        ActorUniqueID       bossID,
        BossEventUpdateType type,
        const string&       title     = "",
        float               health    = 1.0f,
        BossBarColor        color     = BossBarColor::Green,
        BossBarOverlay      overlay   = BossBarOverlay::Progress
    );

    pair<string, BossBarColor> getDimInfo(int dimId) const;
    BossBarOverlay             parseOverlay(const string& value) const;
    int                        countPlayersInDim(Level& level, int dimId) const;
    string                     buildTitle(ServerPlayer& player, PlayerState& state, Level& level, int dimId) const;

    const PluginConfig* mConfig = nullptr;
};
