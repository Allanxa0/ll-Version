#include "TickScheduler.h"

#include "BossBarManager.h"
#include "PlayerRegistry.h"

#include "ll/api/service/Bedrock.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "ll/api/chrono/GameChrono.h"

#include "mc/world/level/Level.h"
#include "mc/server/ServerPlayer.h"

using namespace ll::literals::chrono_literals;

TickScheduler& TickScheduler::getInstance() {
    static TickScheduler instance;
    return instance;
}

void TickScheduler::start(int intervalTicks) {
    mIntervalTicks = intervalTicks;
    scheduleNext();
}

void TickScheduler::stop() {
    if (mCallback) {
        mCallback->cancel();
        mCallback = nullptr;
    }
}

void TickScheduler::tick() {
    auto levelOpt = ll::service::bedrock::getLevel();
    if (!levelOpt) { scheduleNext(); return; }

    Level&    level    = *levelOpt;
    auto&     registry = PlayerRegistry::getInstance();
    auto&     bars     = BossBarManager::getInstance();

    level.forEachPlayer([&](Player& p) -> bool {
        auto* sp    = ServerPlayer::tryGetFromEntity(p.getEntityContext(), false);
        auto* state = sp ? registry.get(sp->getXuid()) : nullptr;

        if (sp && state) bars.update(*sp, *state, level);
        return true;
    });

    scheduleNext();
}

void TickScheduler::scheduleNext() {
    auto& executor = ll::thread::ServerThreadExecutor::getDefault();
    mCallback      = executor.executeAfter(
        [this] { tick(); },
        ll::chrono::ticks(mIntervalTicks)
    );
}
