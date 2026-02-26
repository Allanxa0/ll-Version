#include "glacie/core/block/GlobalBlockPalette.h"
#include "glacie/core/GlacieMod.h"
#include "ll/api/io/Logger.h"
#include "ll/api/mod/NativeMod.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Symbol.h"
#include <fstream>
#include <vector>

GlobalBlockPalette* GlobalBlockP;

void GlobalBlockPalette::init() {
    std::vector<int> protocols = {859, 860, 898, 924};
    std::unordered_map<int, nlohmann::json> jsonMaps;

    for (int protocol : protocols) {
        jsonMaps[protocol] = initMap("BLOCK_INFO_" + std::to_string(protocol));
    }

    auto jsonMain = initMap("BLOCK_INFO_MAIN");

    for (nlohmann::json::iterator it = jsonMain.begin(); it != jsonMain.end(); ++it) {
        for (auto info : it.value()) {
            int data = info["data"];
            auto blocktypename = it.key();

            if (blocktypename == "minecraft:white_stained_glass") blocktypename = "minecraft:stained_glass";
            if (blocktypename == "minecraft:white_stained_glass_pane") blocktypename = "minecraft:stained_glass_pane";
            if (blocktypename == "minecraft:white_terracotta") blocktypename = "minecraft:stained_hardened_clay";
            if (blocktypename == "minecraft:planks") blocktypename = "minecraft:oak_planks";

            for (int protocol : protocols) {
                if (!jsonMaps[protocol].contains(blocktypename)) continue;
                
                auto oldblock = jsonMaps[protocol][blocktypename];
                for (auto oldinfo : oldblock) {
                    int compareData = data;
                    
                    if (blocktypename == "minecraft:chest" || blocktypename == "minecraft:ender_chest"
                        || blocktypename == "minecraft:stonecutter_block" || blocktypename == "minecraft:trapped_chest") {
                        if (compareData == 2) compareData = 2;
                        else if (compareData == 3) compareData = 0;
                        else if (compareData == 4) compareData = 1;
                        else if (compareData == 5) compareData = 3;
                        else continue;
                    }

                    if (oldinfo["data"] == compareData) {
                        RunTimeIdTable[protocol][info["hashruntimeid"]] = oldinfo["hashruntimeid"];
                        RunTimeIdTableOld[protocol][oldinfo["hashruntimeid"]] = info["hashruntimeid"];
                    }
                }
            }
        }
    }
}

uint GlobalBlockPalette::getBlockRuntimeIdFromMain(int protocol, uint oldruntimeid) {
    if (RunTimeIdTable.count(protocol)) {
        if (RunTimeIdTable[protocol].count(oldruntimeid)) { return RunTimeIdTable[protocol][oldruntimeid]; }
    }
    return oldruntimeid;
}

uint GlobalBlockPalette::getBlockRuntimeIdFromOther(int protocol, uint oldruntimeid) {
    if (RunTimeIdTableOld.count(protocol)) {
        if (RunTimeIdTableOld[protocol].count(oldruntimeid)) { return RunTimeIdTableOld[protocol][oldruntimeid]; }
    }
    return oldruntimeid;
}

LL_AUTO_STATIC_HOOK(
    InitFromBlockDefinitions,
    ll::memory::HookPriority::Normal,
    ll::memory::SymbolView("?initFromBlockDefinitions@BlockPalette@@QEAAXXZ"),
    void,
    void* a1
) {
    origin(a1);
    glacie::GlacieMod::getInstance().getSelf().getLogger().info("Glacie is initializing, please wait!");
    auto blockPalette = new GlobalBlockPalette();
    blockPalette->init();
    GlobalBlockP = blockPalette;
    glacie::GlacieMod::getInstance().getSelf().getLogger().info("Glacie initialized successfully!");
}
