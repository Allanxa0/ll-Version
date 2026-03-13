#pragma once

#include <memory>
#include <vector>

#include "ll/api/event/ListenerBase.h"

using namespace std;

class EventHandler {
public:
    static EventHandler& getInstance();

    void registerAll();
    void unregisterAll();

private:
    void onPlayerJoin(class ll::event::player::PlayerJoinEvent& ev);
    void onPlayerDisconnect(class ll::event::player::PlayerDisconnectEvent& ev);
    void onPlayerDie(class ll::event::player::PlayerDieEvent& ev);
    void onPlayerRespawn(class ll::event::player::PlayerRespawnEvent& ev);
    void onPlayerSneak(class ll::event::player::PlayerSneakEvent& ev);
    void onPlayerSprint(class ll::event::player::PlayerSprintEvent& ev);

    vector<shared_ptr<ll::event::ListenerBase>> mListeners;
};
