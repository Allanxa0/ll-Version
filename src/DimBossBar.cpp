#include <memory>

#include "Config.h"
#include "BossBarManager.h"
#include "EventHandler.h"
#include "TickScheduler.h"
#include "CommandHandler.h"
#include "PlayerRegistry.h"

#include "ll/api/mod/NativeMod.h"
#include "ll/api/mod/RegisterHelper.h"
#include "ll/api/io/Logger.h"
#include "ll/api/Config.h"
#include "ll/api/service/Bedrock.h"

using namespace std;

PluginConfig            gConfig;
static ll::io::Logger   gLogger("DimBossBar");

class DimBossBarMod {
public:
    static DimBossBarMod& getInstance() {
        static DimBossBarMod instance;
        return instance;
    }

    bool load(ll::mod::NativeMod& mod) {
        auto path = mod.getConfigDir() / "config.json";
        if (!ll::config::loadConfig(gConfig, path)) {
            ll::config::saveConfig(gConfig, path);
        }
        BossBarManager::getInstance().setConfig(&gConfig);
        return true;
    }

    bool enable(ll::mod::NativeMod& mod) {
        EventHandler::getInstance().registerAll();
        CommandHandler::getInstance().registerAll(mod);
        TickScheduler::getInstance().start(gConfig.updateIntervalTicks);
        gLogger.info("DimBossBar activado.");
        return true;
    }

    bool disable(ll::mod::NativeMod& mod) {
        TickScheduler::getInstance().stop();

        auto levelOpt = ll::service::bedrock::getLevel();
        if (levelOpt) {
            auto& registry = PlayerRegistry::getInstance();
            auto& bars     = BossBarManager::getInstance();

            levelOpt->forEachPlayer([&](Player& p) -> bool {
                auto* sp    = ServerPlayer::tryGetFromEntity(p.getEntityContext(), false);
                auto* state = sp ? registry.get(sp->getXuid()) : nullptr;
                if (sp && state) bars.remove(*sp, *state);
                return true;
            });
        }

        EventHandler::getInstance().unregisterAll();
        PlayerRegistry::getInstance().clear();
        gLogger.info("DimBossBar desactivado.");
        return true;
    }
};

static unique_ptr<DimBossBarMod> gMod;
LL_REGISTER_MOD(DimBossBarMod, gMod);
