#pragma once

#include <string>
#include <ll/api/reflection/Deserialization.h>

namespace bossbar_online {

struct Config {
    int    version           = 1;
    bool   enabled           = true;
    int    updateIntervalSec = 2;       // Seconds between BossBar updates
    float  barPercent        = 1.0f;    // Bar fill percentage (0.0 - 1.0)
    int    barColor          = 1;       // 0=Pink 1=Blue 2=Red 3=Green 4=Yellow 5=Purple 6=White
    std::string barTitle     = "§b§lOnline Players: §e{count}";
    // {count} is replaced at runtime with the actual online player count
};

} // namespace bossbar_online
