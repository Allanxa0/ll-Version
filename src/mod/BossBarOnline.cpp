#include "BossBarOnline.h"

#include <ll/api/Config.h>
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/player/PlayerJoinEvent.h>
#include <ll/api/event/player/PlayerLeaveEvent.h>
#include <ll/api/schedule/Scheduler.h>
#include <ll/api/service/Bedrock.h>

#include <mc/network/packet/BossEventPacket.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/Level.h>

#include <string>
#include <algorithm>
#include <chrono>

namespace bossbar_online {

// Unique BossBar entity ID — we use -1 to avoid conflict with real entities
static constexpr int64_t BOSSBAR_UNIQUE_ID = -0xBBAA01;

static std::unique_ptr<BossBarOnline> instance;

BossBarOnline& BossBarOnline::getInstance() { return *instance; }

// ─────────────────────────────────────────────────
//  Lifecycle: load
// ─────────────────────────────────────────────────
bool BossBarOnline::load() {
    // Load or create default config
    const auto& configPath = getSelf().getConfigDir() / "config.json";
    if (!ll::config::loadConfig(mConfig, configPath)) {
        getSelf().getLogger().warn("Failed to load config, using defaults");
        ll::config::saveConfig(mConfig, configPath);
    }
    getSelf().getLogger().info("BossBar-Online config loaded");
    return true;
}

// ─────────────────────────────────────────────────
//  Lifecycle: enable
// ─────────────────────────────────────────────────
bool BossBarOnline::enable() {
    if (!mConfig.enabled) {
        getSelf().getLogger().info("BossBar-Online is disabled in config");
        return true;
    }

    auto& eventBus = ll::event::EventBus::getInstance();

    // ── Player Join: send BossBar ──
    mPlayerJoinListener = eventBus.emplaceListener<ll::event::player::PlayerJoinEvent>(
        [this](ll::event::player::PlayerJoinEvent& event) {
            auto& player = event.self();
            getSelf().getLogger().info(
                "Sending BossBar to {}", player.getRealName()
            );
            sendBossBarToPlayer(player, true);
        }
    );

    // ── Player Leave: clean up ──
    mPlayerLeaveListener = eventBus.emplaceListener<ll::event::player::PlayerLeaveEvent>(
        [this](ll::event::player::PlayerLeaveEvent& event) {
            auto& player = event.self();
            removeBossBarFromPlayer(player);
        }
    );

    // ── Periodic Scheduler: update BossBar for all players ──
    mScheduler = std::make_unique<ll::schedule::ServerTimeScheduler>();

    auto interval = std::chrono::seconds(mConfig.updateIntervalSec);
    mUpdateTask = mScheduler->add<ll::schedule::RepeatTask>(interval, [this]() {
        updateAllBossBars();
    });

    getSelf().getLogger().info(
        "BossBar-Online enabled! Update interval: {}s",
        mConfig.updateIntervalSec
    );
    return true;
}

// ─────────────────────────────────────────────────
//  Lifecycle: disable
// ─────────────────────────────────────────────────
bool BossBarOnline::disable() {
    auto& eventBus = ll::event::EventBus::getInstance();

    // Remove event listeners
    if (mPlayerJoinListener)  eventBus.removeListener(mPlayerJoinListener);
    if (mPlayerLeaveListener) eventBus.removeListener(mPlayerLeaveListener);

    // Cancel scheduled task
    if (mUpdateTask) {
        mUpdateTask->cancel();
        mUpdateTask = nullptr;
    }
    mScheduler.reset();

    // Remove BossBar from all currently online players
    auto level = ll::service::getLevel();
    if (level) {
        level->forEachPlayer([this](Player& player) -> bool {
            removeBossBarFromPlayer(player);
            return true; // continue iteration
        });
    }

    getSelf().getLogger().info("BossBar-Online disabled");
    return true;
}

// ─────────────────────────────────────────────────
//  Lifecycle: unload
// ─────────────────────────────────────────────────
bool BossBarOnline::unload() {
    getSelf().getLogger().info("BossBar-Online unloaded");
    return true;
}

// ─────────────────────────────────────────────────
//  Send / Show BossBar to a player
// ─────────────────────────────────────────────────
void BossBarOnline::sendBossBarToPlayer(Player& player, bool show) {
    auto level = ll::service::getLevel();
    if (!level) return;

    // Count online players
    int onlineCount = 0;
    level->forEachPlayer([&onlineCount](Player&) -> bool {
        onlineCount++;
        return true;
    });

    std::string title = formatTitle(onlineCount);
    float       pct   = std::clamp(mConfig.barPercent, 0.0f, 1.0f);

    if (show) {
        // Send SHOW event — creates the BossBar on the client
        BossEventPacket showPkt;
        showPkt.mBossID        = ActorUniqueID(BOSSBAR_UNIQUE_ID);
        showPkt.mPlayerID      = player.getOrCreateUniqueID();
        showPkt.mType          = BossEventPacket::BossEventUpdateType::Show;
        showPkt.mName          = title;
        showPkt.mHealthPercent = pct;
        showPkt.mColor         = static_cast<BossBarColor>(mConfig.barColor);
        showPkt.mOverlay       = BossBarOverlay::Progress;
        showPkt.mDarkenScreen  = 0;
        player.sendNetworkPacket(showPkt);
    } else {
        // Send UPDATE events — updates title and percentage
        BossEventPacket updateNamePkt;
        updateNamePkt.mBossID        = ActorUniqueID(BOSSBAR_UNIQUE_ID);
        updateNamePkt.mType          = BossEventPacket::BossEventUpdateType::UpdateName;
        updateNamePkt.mName          = title;
        player.sendNetworkPacket(updateNamePkt);

        BossEventPacket updatePctPkt;
        updatePctPkt.mBossID         = ActorUniqueID(BOSSBAR_UNIQUE_ID);
        updatePctPkt.mType           = BossEventPacket::BossEventUpdateType::UpdatePercent;
        updatePctPkt.mHealthPercent  = pct;
        player.sendNetworkPacket(updatePctPkt);
    }
}

// ─────────────────────────────────────────────────
//  Remove / Hide BossBar from a player
// ─────────────────────────────────────────────────
void BossBarOnline::removeBossBarFromPlayer(Player& player) {
    BossEventPacket hidePkt;
    hidePkt.mBossID   = ActorUniqueID(BOSSBAR_UNIQUE_ID);
    hidePkt.mPlayerID = player.getOrCreateUniqueID();
    hidePkt.mType     = BossEventPacket::BossEventUpdateType::Hide;
    player.sendNetworkPacket(hidePkt);
}

// ─────────────────────────────────────────────────
//  Update BossBar for all online players
// ─────────────────────────────────────────────────
void BossBarOnline::updateAllBossBars() {
    auto level = ll::service::getLevel();
    if (!level) return;

    level->forEachPlayer([this](Player& player) -> bool {
        sendBossBarToPlayer(player, false); // update, not show
        return true;
    });
}

// ─────────────────────────────────────────────────
//  Format the title string, replacing {count}
// ─────────────────────────────────────────────────
std::string BossBarOnline::formatTitle(int onlineCount) {
    std::string result = mConfig.barTitle;
    std::string placeholder = "{count}";

    auto pos = result.find(placeholder);
    if (pos != std::string::npos) {
        result.replace(pos, placeholder.length(), std::to_string(onlineCount));
    }
    return result;
}

// ─────────────────────────────────────────────────
//  Register the mod with LeviLamina
// ─────────────────────────────────────────────────
LL_REGISTER_MOD(BossBarOnline, instance);

} // namespace bossbar_online
