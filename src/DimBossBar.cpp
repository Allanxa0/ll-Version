/*
 * DimBossBar - Plugin de BossBar por Dimensiones para LeviLamina
 *
 * Muestra en la BossBar:
 *   - Nombre de la dimensión con color único
 *   - Coordenadas X Y Z en tiempo real
 *   - Estado de sprint / sneak
 *   - Contador de muertes del jugador
 *   - Cantidad de jugadores en cada dimensión
 *
 * Comandos:
 *   /dimbar reload   — recarga la configuración sin reiniciar
 *   /dimbar toggle   — activa/desactiva la barra para ese jugador
 *   /dimbar info     — muestra estadísticas de dimensiones
 */

#include "DimBossBar.h"

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// LL API — eventos
#include "ll/api/event/EventBus.h"
#include "ll/api/event/Listener.h"
#include "ll/api/event/player/PlayerDieEvent.h"
#include "ll/api/event/player/PlayerJoinEvent.h"
#include "ll/api/event/player/PlayerDisconnectEvent.h"
#include "ll/api/event/player/PlayerRespawnEvent.h"
#include "ll/api/event/player/PlayerSneakEvent.h"
#include "ll/api/event/player/PlayerSprintEvent.h"

// LL API — servicio y comandos
#include "ll/api/service/Bedrock.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/mod/NativeMod.h"
#include "ll/api/io/Logger.h"
#include "ll/api/Config.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "ll/api/chrono/GameChrono.h"

// MC — jugadores y paquetes
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/level/Level.h"
#include "mc/network/packet/BossEventPacket.h"
#include "mc/world/actor/ai/util/BossBarColor.h"
#include "mc/world/actor/ai/util/BossBarOverlay.h"
#include "mc/world/actor/ai/util/BossEventUpdateType.h"
#include "mc/legacy/ActorUniqueID.h"

using namespace ll::literals::chrono_literals;

// ─────────────────────────────────────────────────────────────────────────────
//  Configuración serializable
// ─────────────────────────────────────────────────────────────────────────────
struct DimBossBarConfig {
    int version = 1;

    // Intervalo de actualización de la barra (en ticks, 20 ticks = 1 segundo)
    int updateIntervalTicks = 20;

    // Textos personalizables por dimensión (soportan §-codes)
    std::string overworldName = "§a🌍 Overworld";
    std::string netherName    = "§c🔥 Nether";
    std::string endName       = "§5🌑 The End";
    std::string unknownName   = "§7❓ Unknown";

    // Mostrar coordenadas
    bool showCoords  = true;
    // Mostrar estado sprint/sneak
    bool showStatus  = true;
    // Mostrar muertes
    bool showDeaths  = true;
    // Mostrar jugadores en la misma dimensión
    bool showDimPop  = true;

    // Overlay de la barra (Progress, Notched6, Notched10, Notched12, Notched20)
    std::string barOverlay = "Progress";
};

// ─────────────────────────────────────────────────────────────────────────────
//  Estado por jugador
// ─────────────────────────────────────────────────────────────────────────────
struct PlayerState {
    int  deaths      = 0;
    bool barEnabled  = true;
    bool isSneaking  = false;
    bool isSprinting = false;

    // ID único de la bossbar de este jugador (usamos su propio ActorUniqueID)
    ActorUniqueID bossID{0};
    bool          barCreated = false;
};

// ─────────────────────────────────────────────────────────────────────────────
//  Instancia del mod (singleton simple)
// ─────────────────────────────────────────────────────────────────────────────
static DimBossBarConfig                              gConfig;
static std::unordered_map<std::string, PlayerState>  gPlayerStates; // xuid → state
static ll::io::Logger                                gLogger("DimBossBar");

// Listeners registrados (guardados para poder desregistrarlos)
static std::vector<std::shared_ptr<ll::event::ListenerBase>> gListeners;

// Callback de actualización periódica (cancellable)
static std::shared_ptr<ll::data::CancellableCallback> gTickCallback;

// ─────────────────────────────────────────────────────────────────────────────
//  Utilidades
// ─────────────────────────────────────────────────────────────────────────────

// Convierte DimensionType (int) → nombre y color de barra
static std::pair<std::string, BossBarColor> getDimInfo(int dimId) {
    switch (dimId) {
    case 0: // Overworld
        return {gConfig.overworldName, BossBarColor::Green};
    case 1: // Nether
        return {gConfig.netherName, BossBarColor::Red};
    case 2: // The End
        return {gConfig.endName, BossBarColor::Purple};
    default:
        return {gConfig.unknownName, BossBarColor::White};
    }
}

// Convierte el string de config al enum BossBarOverlay
static BossBarOverlay parseOverlay(std::string const& s) {
    if (s == "Notched6")  return BossBarOverlay::Notched6;
    if (s == "Notched10") return BossBarOverlay::Notched10;
    if (s == "Notched12") return BossBarOverlay::Notched12;
    if (s == "Notched20") return BossBarOverlay::Notched20;
    return BossBarOverlay::Progress;
}

// Cuenta jugadores en una dimensión concreta
static int countPlayersInDim(Level& level, int targetDimId) {
    int count = 0;
    level.forEachPlayer([&](Player& p) -> bool {
        if (p.getDimensionId() == targetDimId) ++count;
        return true; // continuar iteración
    });
    return count;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Envío de BossEventPacket
// ─────────────────────────────────────────────────────────────────────────────

static void sendBossEvent(
    ServerPlayer&       player,
    ActorUniqueID       bossID,
    BossEventUpdateType type,
    std::string const&  title       = "",
    float               healthPct   = 1.0f,
    BossBarColor        color       = BossBarColor::Green,
    BossBarOverlay      overlay     = BossBarOverlay::Progress
) {
    BossEventPacket pkt;
    pkt.mBossID        = bossID;
    pkt.mPlayerID      = player.getOrCreateUniqueID();
    pkt.mEventType     = type;
    pkt.mName          = title;
    pkt.mHealthPercent = healthPct;
    pkt.mColor         = color;
    pkt.mOverlay       = overlay;
    pkt.mDarkenScreen  = 0;
    pkt.mCreateWorldFog= 0;
    player.sendNetworkPacket(pkt);
}

// Crea la barra (ADD) para un jugador
static void createBar(ServerPlayer& player, PlayerState& state) {
    auto bossID = player.getOrCreateUniqueID();
    state.bossID   = bossID;
    state.barCreated = true;

    auto [dimName, color] = getDimInfo(player.getDimensionId());
    auto overlay          = parseOverlay(gConfig.barOverlay);

    sendBossEvent(player, bossID, BossEventUpdateType::Add,
                  dimName, 1.0f, color, overlay);
}

// Elimina la barra (REMOVE) para un jugador
static void removeBar(ServerPlayer& player, PlayerState& state) {
    if (!state.barCreated) return;
    sendBossEvent(player, state.bossID, BossEventUpdateType::Remove);
    state.barCreated = false;
}

// Actualiza título + color de la barra existente
static void updateBar(ServerPlayer& player, PlayerState& state, Level& level) {
    if (!state.barEnabled) return;

    int  dimId            = player.getDimensionId();
    auto [dimName, color] = getDimInfo(dimId);
    auto overlay          = parseOverlay(gConfig.barOverlay);

    // ── Construir el texto de la bossbar ──────────────────────────────────
    std::string text = dimName;

    if (gConfig.showCoords) {
        auto pos = player.getPosition();
        text += " §7| §fX:" + std::to_string((int)pos.x)
              + " Y:" + std::to_string((int)pos.y)
              + " Z:" + std::to_string((int)pos.z);
    }

    if (gConfig.showStatus) {
        if (state.isSneaking)       text += " §e[SNEAK]";
        else if (state.isSprinting) text += " §b[SPRINT]";
    }

    if (gConfig.showDeaths) {
        text += " §c💀" + std::to_string(state.deaths);
    }

    if (gConfig.showDimPop) {
        int dimPop = countPlayersInDim(level, dimId);
        text += " §a👥" + std::to_string(dimPop);
    }

    // Si la barra no se había creado aún, la creamos ahora
    if (!state.barCreated) {
        createBar(player, state);
    }

    // UPDATE_NAME — cambia el título
    sendBossEvent(player, state.bossID, BossEventUpdateType::UpdateName,
                  text, 1.0f, color, overlay);

    // UPDATE_PROPERTIES — cambia color y overlay
    sendBossEvent(player, state.bossID, BossEventUpdateType::UpdateProperties,
                  "", 1.0f, color, overlay);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Tick periódico de actualización
// ─────────────────────────────────────────────────────────────────────────────
static void scheduleTick();

static void onTick() {
    auto levelOpt = ll::service::bedrock::getLevel();
    if (!levelOpt) { scheduleTick(); return; }
    Level& level = *levelOpt;

    level.forEachPlayer([&](Player& p) -> bool {
        // Necesitamos un ServerPlayer para sendNetworkPacket
        auto* sp = ServerPlayer::tryGetFromEntity(p.getEntityContext(), false);
        if (!sp) return true;

        std::string xuid = sp->getXuid();
        auto it = gPlayerStates.find(xuid);
        if (it == gPlayerStates.end()) return true;

        PlayerState& state = it->second;
        if (state.barEnabled) {
            updateBar(*sp, state, level);
        }
        return true;
    });

    scheduleTick();
}

static void scheduleTick() {
    using namespace ll::chrono;
    auto intervalTicks = ll::chrono::ticks(gConfig.updateIntervalTicks);

    auto& executor = ll::thread::ServerThreadExecutor::getDefault();
    gTickCallback  = executor.executeAfter(onTick, intervalTicks);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Registro de eventos
// ─────────────────────────────────────────────────────────────────────────────
static void registerEvents() {
    auto& bus = ll::event::EventBus::getInstance();

    // ── PlayerJoin ────────────────────────────────────────────────────────
    auto joinListener = ll::event::Listener<ll::event::player::PlayerJoinEvent>::create(
        [](ll::event::player::PlayerJoinEvent& ev) {
            auto* sp = ServerPlayer::tryGetFromEntity(ev.self().getEntityContext(), false);
            if (!sp) return;
            std::string xuid = sp->getXuid();
            if (gPlayerStates.find(xuid) == gPlayerStates.end()) {
                gPlayerStates[xuid] = PlayerState{};
            }
            gLogger.info("DimBossBar: jugador {} unido.", sp->getRealName());
        }
    );
    bus.addListener<ll::event::player::PlayerJoinEvent>(joinListener);
    gListeners.push_back(joinListener);

    // ── PlayerDisconnect ──────────────────────────────────────────────────
    auto quitListener = ll::event::Listener<ll::event::player::PlayerDisconnectEvent>::create(
        [](ll::event::player::PlayerDisconnectEvent& ev) {
            auto* sp = ServerPlayer::tryGetFromEntity(ev.self().getEntityContext(), false);
            if (!sp) return;
            std::string xuid = sp->getXuid();
            // Eliminar barra antes de desconectar
            auto it = gPlayerStates.find(xuid);
            if (it != gPlayerStates.end()) {
                removeBar(*sp, it->second);
                // Conservamos estado (muertes) para reconexión
            }
        }
    );
    bus.addListener<ll::event::player::PlayerDisconnectEvent>(quitListener);
    gListeners.push_back(quitListener);

    // ── PlayerDie ─────────────────────────────────────────────────────────
    auto dieListener = ll::event::Listener<ll::event::player::PlayerDieEvent>::create(
        [](ll::event::player::PlayerDieEvent& ev) {
            std::string xuid = ev.self().getXuid();
            auto it = gPlayerStates.find(xuid);
            if (it != gPlayerStates.end()) {
                ++it->second.deaths;
            }
        }
    );
    bus.addListener<ll::event::player::PlayerDieEvent>(dieListener);
    gListeners.push_back(dieListener);

    // ── PlayerRespawn ─────────────────────────────────────────────────────
    auto respawnListener = ll::event::Listener<ll::event::player::PlayerRespawnEvent>::create(
        [](ll::event::player::PlayerRespawnEvent& ev) {
            auto* sp = ServerPlayer::tryGetFromEntity(ev.self().getEntityContext(), false);
            if (!sp) return;
            std::string xuid = sp->getXuid();
            auto it = gPlayerStates.find(xuid);
            if (it != gPlayerStates.end()) {
                // Recrear la barra tras el respawn
                it->second.barCreated = false;
                createBar(*sp, it->second);
            }
        }
    );
    bus.addListener<ll::event::player::PlayerRespawnEvent>(respawnListener);
    gListeners.push_back(respawnListener);

    // ── PlayerSneak ───────────────────────────────────────────────────────
    auto sneakListener = ll::event::Listener<ll::event::player::PlayerSneakEvent>::create(
        [](ll::event::player::PlayerSneakEvent& ev) {
            std::string xuid = ev.self().getXuid();
            auto it = gPlayerStates.find(xuid);
            if (it != gPlayerStates.end()) {
                it->second.isSneaking  = ev.isSneaking();
                if (ev.isSneaking()) it->second.isSprinting = false;
            }
        }
    );
    bus.addListener<ll::event::player::PlayerSneakEvent>(sneakListener);
    gListeners.push_back(sneakListener);

    // ── PlayerSprint ──────────────────────────────────────────────────────
    auto sprintListener = ll::event::Listener<ll::event::player::PlayerSprintEvent>::create(
        [](ll::event::player::PlayerSprintEvent& ev) {
            std::string xuid = ev.self().getXuid();
            auto it = gPlayerStates.find(xuid);
            if (it != gPlayerStates.end()) {
                it->second.isSprinting = ev.isSprinting();
                if (ev.isSprinting()) it->second.isSneaking = false;
            }
        }
    );
    bus.addListener<ll::event::player::PlayerSprintEvent>(sprintListener);
    gListeners.push_back(sprintListener);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Comando /dimbar
// ─────────────────────────────────────────────────────────────────────────────

struct DimBarReloadParam  {};  // /dimbar reload
struct DimBarToggleParam  {};  // /dimbar toggle
struct DimBarInfoParam    {};  // /dimbar info

static void registerCommands(ll::mod::NativeMod& mod) {
    auto& reg = ll::command::CommandRegistrar::getInstance(false);

    // ── /dimbar reload ────────────────────────────────────────────────────
    reg.getOrCreateCommand("dimbar", "DimBossBar — gestión de la bossbar por dimensiones",
                           CommandPermissionLevel::GameDirectors)
        .overload<DimBarReloadParam>()
        .text("reload")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, DimBarReloadParam const&) {
            auto cfgPath = mod.getConfigDir() / "config.json";
            if (ll::config::loadConfig(gConfig, cfgPath)) {
                output.success("§aDimBossBar: configuración recargada.");
            } else {
                output.error("§cDimBossBar: no se pudo recargar la configuración.");
            }
        });

    // ── /dimbar toggle ────────────────────────────────────────────────────
    reg.getOrCreateCommand("dimbar", "DimBossBar — gestión de la bossbar por dimensiones",
                           CommandPermissionLevel::Any)
        .overload<DimBarToggleParam>()
        .text("toggle")
        .execute([](CommandOrigin const& origin, CommandOutput& output, DimBarToggleParam const&) {
            auto* entity = origin.getEntity();
            if (!entity) { output.error("§cSolo jugadores pueden usar este comando."); return; }
            auto* sp = static_cast<ServerPlayer*>(entity);
            std::string xuid = sp->getXuid();

            auto it = gPlayerStates.find(xuid);
            if (it == gPlayerStates.end()) {
                output.error("§cEstado de jugador no encontrado."); return;
            }

            it->second.barEnabled = !it->second.barEnabled;
            if (!it->second.barEnabled) {
                removeBar(*sp, it->second);
                output.success("§eBossBar §cdesactivada§e.");
            } else {
                createBar(*sp, it->second);
                output.success("§eBossBar §aactivada§e.");
            }
        });

    // ── /dimbar info ──────────────────────────────────────────────────────
    reg.getOrCreateCommand("dimbar", "DimBossBar — gestión de la bossbar por dimensiones",
                           CommandPermissionLevel::Any)
        .overload<DimBarInfoParam>()
        .text("info")
        .execute([](CommandOrigin const& origin, CommandOutput& output, DimBarInfoParam const&) {
            auto levelOpt = ll::service::bedrock::getLevel();
            if (!levelOpt) { output.error("§cNivel no disponible."); return; }
            Level& level = *levelOpt;

            int ow  = countPlayersInDim(level, 0);
            int nth = countPlayersInDim(level, 1);
            int end = countPlayersInDim(level, 2);

            output.success("§6=== DimBossBar Info ===");
            output.success("§a🌍 Overworld: §f" + std::to_string(ow)  + " jugadores");
            output.success("§c🔥 Nether:    §f" + std::to_string(nth) + " jugadores");
            output.success("§5🌑 The End:   §f" + std::to_string(end) + " jugadores");
        });
}

// ─────────────────────────────────────────────────────────────────────────────
//  Punto de entrada del mod
// ─────────────────────────────────────────────────────────────────────────────
namespace dim_bossbar {

static std::unique_ptr<DimBossBarMod> gMod;

DimBossBarMod& DimBossBarMod::getInstance() { return *gMod; }

bool DimBossBarMod::load(ll::mod::NativeMod& mod) {
    gLogger.info("DimBossBar cargando...");

    // Cargar o crear configuración
    auto cfgPath = mod.getConfigDir() / "config.json";
    if (!ll::config::loadConfig(gConfig, cfgPath)) {
        gLogger.warn("Configuración no encontrada, creando por defecto...");
        ll::config::saveConfig(gConfig, cfgPath);
    }

    return true;
}

bool DimBossBarMod::enable(ll::mod::NativeMod& mod) {
    gLogger.info("DimBossBar activando...");

    registerEvents();
    registerCommands(mod);
    scheduleTick();

    gLogger.info("DimBossBar activado. Intervalo: {} ticks.", gConfig.updateIntervalTicks);
    return true;
}

bool DimBossBarMod::disable(ll::mod::NativeMod& mod) {
    gLogger.info("DimBossBar desactivando...");

    // Cancelar el tick periódico
    if (gTickCallback) { gTickCallback->cancel(); gTickCallback = nullptr; }

    // Eliminar barras de todos los jugadores conectados
    auto levelOpt = ll::service::bedrock::getLevel();
    if (levelOpt) {
        levelOpt->forEachPlayer([&](Player& p) -> bool {
            auto* sp = ServerPlayer::tryGetFromEntity(p.getEntityContext(), false);
            if (!sp) return true;
            auto it = gPlayerStates.find(sp->getXuid());
            if (it != gPlayerStates.end()) removeBar(*sp, it->second);
            return true;
        });
    }

    // Limpiar listeners
    auto& bus = ll::event::EventBus::getInstance();
    for (auto& l : gListeners) bus.removeListener(l);
    gListeners.clear();

    gPlayerStates.clear();
    return true;
}

} // namespace dim_bossbar

// ─────────────────────────────────────────────────────────────────────────────
//  Hook de entrada LeviLamina (LL_REGISTER_MOD)
// ─────────────────────────────────────────────────────────────────────────────
LL_REGISTER_MOD(dim_bossbar::DimBossBarMod, dim_bossbar::gMod);
