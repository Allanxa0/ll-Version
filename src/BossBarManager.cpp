#include "BossBarManager.h"

#include "mc/network/packet/BossEventPacket.h"
#include "mc/world/actor/Actor.h"

BossBarManager& BossBarManager::getInstance() {
    static BossBarManager instance;
    return instance;
}

void BossBarManager::setConfig(const PluginConfig* config) {
    mConfig = config;
}

void BossBarManager::create(ServerPlayer& player, PlayerState& state) {
    auto [dimName, color] = getDimInfo(player.getDimensionId());
    state.bossID          = player.getOrCreateUniqueID();
    state.barCreated      = true;

    send(player, state.bossID, BossEventUpdateType::Add, dimName, 1.0f, color, parseOverlay(mConfig->barOverlay));
}

void BossBarManager::remove(ServerPlayer& player, PlayerState& state) {
    if (!state.barCreated) return;
    send(player, state.bossID, BossEventUpdateType::Remove);
    state.barCreated = false;
}

void BossBarManager::update(ServerPlayer& player, PlayerState& state, Level& level) {
    if (!state.barEnabled) return;

    int  dimId            = player.getDimensionId();
    auto [dimName, color] = getDimInfo(dimId);
    auto overlay          = parseOverlay(mConfig->barOverlay);
    auto title            = buildTitle(player, state, level, dimId);

    if (!state.barCreated) create(player, state);

    send(player, state.bossID, BossEventUpdateType::UpdateName,       title, 1.0f, color, overlay);
    send(player, state.bossID, BossEventUpdateType::UpdateProperties, "",    1.0f, color, overlay);
}

void BossBarManager::send(
    ServerPlayer&       player,
    ActorUniqueID       bossID,
    BossEventUpdateType type,
    const string&       title,
    float               health,
    BossBarColor        color,
    BossBarOverlay      overlay
) {
    BossEventPacket pkt(false);
    pkt.mBossID         = bossID;
    pkt.mPlayerID       = player.getOrCreateUniqueID();
    pkt.mEventType      = type;
    pkt.mName           = title;
    pkt.mHealthPercent  = health;
    pkt.mColor          = color;
    pkt.mOverlay        = overlay;
    pkt.mDarkenScreen   = 0;
    pkt.mCreateWorldFog = 0;
    player.sendNetworkPacket(pkt);
}

pair<string, BossBarColor> BossBarManager::getDimInfo(int dimId) const {
    switch (dimId) {
    case 0:  return { mConfig->overworldName, BossBarColor::Green  };
    case 1:  return { mConfig->netherName,    BossBarColor::Red    };
    case 2:  return { mConfig->endName,       BossBarColor::Purple };
    default: return { mConfig->unknownName,   BossBarColor::White  };
    }
}

BossBarOverlay BossBarManager::parseOverlay(const string& value) const {
    if (value == "Notched6")  return BossBarOverlay::Notched6;
    if (value == "Notched10") return BossBarOverlay::Notched10;
    if (value == "Notched12") return BossBarOverlay::Notched12;
    if (value == "Notched20") return BossBarOverlay::Notched20;
    return BossBarOverlay::Progress;
}

int BossBarManager::countPlayersInDim(Level& level, int dimId) const {
    int count = 0;
    level.forEachPlayer([&](Player& p) -> bool {
        if (p.getDimensionId() == dimId) ++count;
        return true;
    });
    return count;
}

string BossBarManager::buildTitle(ServerPlayer& player, PlayerState& state, Level& level, int dimId) const {
    auto [dimName, _] = getDimInfo(dimId);
    string title      = dimName;

    if (mConfig->showCoords) {
        auto pos = player.getPosition();
        title += " \xa7\x37| \xa7\x66X:" + to_string((int)pos.x)
               + " Y:"                   + to_string((int)pos.y)
               + " Z:"                   + to_string((int)pos.z);
    }

    if (mConfig->showStatus) {
        if      (state.isSneaking)  title += " \xa7\x65[SNEAK]";
        else if (state.isSprinting) title += " \xa7\x62[SPRINT]";
    }

    if (mConfig->showDeaths) {
        title += " \xa7\x63\xf0\x9f\x92\x80" + to_string(state.deaths);
    }

    if (mConfig->showDimPop) {
        title += " \xa7\x61\xf0\x9f\x91\xa5" + to_string(countPlayersInDim(level, dimId));
    }

    return title;
}
