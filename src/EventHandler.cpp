#include "EventHandler.h"

#include "BossBarManager.h"
#include "PlayerRegistry.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/Listener.h"
#include "ll/api/event/player/PlayerJoinEvent.h"
#include "ll/api/event/player/PlayerDisconnectEvent.h"
#include "ll/api/event/player/PlayerDieEvent.h"
#include "ll/api/event/player/PlayerRespawnEvent.h"
#include "ll/api/event/player/PlayerSneakEvent.h"
#include "ll/api/event/player/PlayerSprintEvent.h"

#include "mc/server/ServerPlayer.h"

using namespace ll::event;
using namespace ll::event::player;

EventHandler& EventHandler::getInstance() {
    static EventHandler instance;
    return instance;
}

void EventHandler::registerAll() {
    auto& bus      = EventBus::getInstance();
    auto& registry = PlayerRegistry::getInstance();
    auto& bars     = BossBarManager::getInstance();

    auto join = Listener<PlayerJoinEvent>::create([&](PlayerJoinEvent& ev) {
        onPlayerJoin(ev);
    });
    auto quit = Listener<PlayerDisconnectEvent>::create([&](PlayerDisconnectEvent& ev) {
        onPlayerDisconnect(ev);
    });
    auto die = Listener<PlayerDieEvent>::create([&](PlayerDieEvent& ev) {
        onPlayerDie(ev);
    });
    auto respawn = Listener<PlayerRespawnEvent>::create([&](PlayerRespawnEvent& ev) {
        onPlayerRespawn(ev);
    });
    auto sneak = Listener<PlayerSneakEvent>::create([&](PlayerSneakEvent& ev) {
        onPlayerSneak(ev);
    });
    auto sprint = Listener<PlayerSprintEvent>::create([&](PlayerSprintEvent& ev) {
        onPlayerSprint(ev);
    });

    bus.addListener<PlayerJoinEvent>(join);
    bus.addListener<PlayerDisconnectEvent>(quit);
    bus.addListener<PlayerDieEvent>(die);
    bus.addListener<PlayerRespawnEvent>(respawn);
    bus.addListener<PlayerSneakEvent>(sneak);
    bus.addListener<PlayerSprintEvent>(sprint);

    mListeners = { join, quit, die, respawn, sneak, sprint };
}

void EventHandler::unregisterAll() {
    auto& bus = EventBus::getInstance();
    for (auto& listener : mListeners) bus.removeListener(listener);
    mListeners.clear();
}

void EventHandler::onPlayerJoin(PlayerJoinEvent& ev) {
    auto* sp = ServerPlayer::tryGetFromEntity(ev.self().getEntityContext(), false);
    if (!sp) return;
    PlayerRegistry::getInstance().getOrCreate(sp->getXuid());
}

void EventHandler::onPlayerDisconnect(PlayerDisconnectEvent& ev) {
    auto* sp = ServerPlayer::tryGetFromEntity(ev.self().getEntityContext(), false);
    if (!sp) return;

    auto& registry = PlayerRegistry::getInstance();
    auto* state    = registry.get(sp->getXuid());
    if (state) BossBarManager::getInstance().remove(*sp, *state);
}

void EventHandler::onPlayerDie(PlayerDieEvent& ev) {
    auto* state = PlayerRegistry::getInstance().get(ev.self().getXuid());
    if (state) ++state->deaths;
}

void EventHandler::onPlayerRespawn(PlayerRespawnEvent& ev) {
    auto* sp = ServerPlayer::tryGetFromEntity(ev.self().getEntityContext(), false);
    if (!sp) return;

    auto* state = PlayerRegistry::getInstance().get(sp->getXuid());
    if (!state) return;

    state->barCreated = false;
    BossBarManager::getInstance().create(*sp, *state);
}

void EventHandler::onPlayerSneak(PlayerSneakEvent& ev) {
    auto* state = PlayerRegistry::getInstance().get(ev.self().getXuid());
    if (!state) return;

    state->isSneaking = ev.isSneaking();
    if (state->isSneaking) state->isSprinting = false;
}

void EventHandler::onPlayerSprint(PlayerSprintEvent& ev) {
    auto* state = PlayerRegistry::getInstance().get(ev.self().getXuid());
    if (!state) return;

    state->isSprinting = ev.isSprinting();
    if (state->isSprinting) state->isSneaking = false;
}
