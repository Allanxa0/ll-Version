#pragma once

#include "Config.h"

#include <ll/api/mod/NativeMod.h>
#include <ll/api/event/Listener.h>
#include <ll/api/schedule/Scheduler.h>

#include <memory>
#include <atomic>

namespace bossbar_online {

class BossBarOnline {

public:
    static BossBarOnline& getInstance();

    BossBarOnline(ll::mod::NativeMod& self) : mSelf(self) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    bool load();
    bool enable();
    bool disable();
    bool unload();

private:
    ll::mod::NativeMod& mSelf;
    Config              mConfig;

    // Event listeners
    ll::event::ListenerPtr mPlayerJoinListener;
    ll::event::ListenerPtr mPlayerLeaveListener;

    // Scheduler for periodic BossBar updates
    std::unique_ptr<ll::schedule::ServerTimeScheduler> mScheduler;
    ll::schedule::ServerTimeScheduler::TaskPtr         mUpdateTask;

    // Helpers
    void sendBossBarToPlayer(class Player& player, bool show);
    void removeBossBarFromPlayer(class Player& player);
    void updateAllBossBars();
    std::string formatTitle(int onlineCount);
};

} // namespace bossbar_online
