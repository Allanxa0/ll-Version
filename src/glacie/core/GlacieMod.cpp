#include "GlacieMod.h"
#include "ll/api/mod/RegisterHelper.h"

namespace glacie {

GlacieMod& GlacieMod::getInstance() {
    static GlacieMod instance;
    return instance;
}

bool GlacieMod::load() {
    getSelf().getLogger().info("Loading Glacie...");
    return true;
}

bool GlacieMod::enable() {
    getSelf().getLogger().info("  _____ _            _    ");
    getSelf().getLogger().info(" / ____| |          (_)  ");
    getSelf().getLogger().info("| |  __| | __ _  ___ _  ___");
    getSelf().getLogger().info("Glacie enabled successfully!");
    return true;
}

bool GlacieMod::disable() {
    getSelf().getLogger().info("Disabling Glacie...");
    return true;
}

}

LL_REGISTER_MOD(glacie::GlacieMod, glacie::GlacieMod::getInstance());