#include "CommandHandler.h"

#include "BossBarManager.h"
#include "PlayerRegistry.h"

#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/Config.h"

#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/level/Level.h"

using namespace ll::command;
using namespace std;

struct ReloadParam  {};
struct ToggleParam  {};
struct InfoParam    {};

CommandHandler& CommandHandler::getInstance() {
    static CommandHandler instance;
    return instance;
}

void CommandHandler::registerAll(ll::mod::NativeMod& mod) {
    auto& reg = CommandRegistrar::getInstance(false);

    auto& cmd = reg.getOrCreateCommand(
        "dimbar",
        "DimBossBar — bossbar dinámica por dimensiones",
        CommandPermissionLevel::Any
    );

    cmd.overload<ReloadParam>()
        .text("reload")
        .execute([&mod](CommandOrigin const&, CommandOutput& output, ReloadParam const&) {
            extern PluginConfig gConfig;
            auto path = mod.getConfigDir() / "config.json";
            if (ll::config::loadConfig(gConfig, path)) {
                output.success("§aDimBossBar: configuración recargada.");
            } else {
                output.error("§cDimBossBar: error al recargar la configuración.");
            }
        });

    cmd.overload<ToggleParam>()
        .text("toggle")
        .execute([](CommandOrigin const& origin, CommandOutput& output, ToggleParam const&) {
            auto* entity = origin.getEntity();
            if (!entity) { output.error("§cSolo jugadores."); return; }

            auto* sp    = static_cast<ServerPlayer*>(entity);
            auto* state = PlayerRegistry::getInstance().get(sp->getXuid());
            if (!state) { output.error("§cEstado no encontrado."); return; }

            state->barEnabled = !state->barEnabled;

            if (!state->barEnabled) {
                BossBarManager::getInstance().remove(*sp, *state);
                output.success("§eBossBar §cdesactivada§e.");
            } else {
                BossBarManager::getInstance().create(*sp, *state);
                output.success("§eBossBar §aactivada§e.");
            }
        });

    cmd.overload<InfoParam>()
        .text("info")
        .execute([](CommandOrigin const&, CommandOutput& output, InfoParam const&) {
            auto levelOpt = ll::service::bedrock::getLevel();
            if (!levelOpt) { output.error("§cNivel no disponible."); return; }

            Level& level = *levelOpt;
            auto count   = [&](int id) {
                int n = 0;
                level.forEachPlayer([&](Player& p) -> bool {
                    if (p.getDimensionId() == id) ++n;
                    return true;
                });
                return n;
            };

            output.success("§6=== DimBossBar Info ===");
            output.success("§a🌍 Overworld §f" + to_string(count(0)) + " jugadores");
            output.success("§c🔥 Nether    §f" + to_string(count(1)) + " jugadores");
            output.success("§5🌑 The End   §f" + to_string(count(2)) + " jugadores");
        });
}
