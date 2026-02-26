#include "glacie/core/block/GlobalBlockPalette.h"
#include "ll/api/io/Logger.h"
#include "ll/core/LeviLamina.h"
#include "ll/api/memory/Hook.h"
#include <fstream>

GlobalBlockPalette* GlobalBlockP;

void GlobalBlockPalette::init() {
    auto json776 = initMap(L"BLOCK_INFO_1_21_60");
    auto json766 = initMap(L"BLOCK_INFO_1_21_50");
    auto json748 = initMap(L"BLOCK_INFO_1_21_40");

    for (nlohmann::json::iterator it = json748.begin(); it != json748.end(); ++it) {
        for (auto info : it.value()) {
            {
                int  data          = info["data"];
                auto blocktypename = it.key();
                if (blocktypename == "minecraft:white_stained_glass") { blocktypename = "minecraft:stained_glass"; }
                if (blocktypename == "minecraft:white_stained_glass_pane") {
                    blocktypename = "minecraft:stained_glass_pane";
                }
                if (blocktypename == "minecraft:white_terracotta") {
                    blocktypename = "minecraft:stained_hardened_clay";
                }

                auto oldblock729 = json729[blocktypename];
                for (auto oldinfo729 : oldblock729) {
                    if (oldinfo729["data"] == data) {
                        RunTimeIdTable[729][info["hashruntimeid"]]          = oldinfo729["hashruntimeid"];
                        RunTimeIdTableOld[729][oldinfo729["hashruntimeid"]] = info["hashruntimeid"];
                    }
                }
            }

            {
                auto oldblock766 = json766[it.key()];
                for (auto oldinfo766 : oldblock766) {
                    int data = info["data"];
                    if (it.key() == "minecraft:chest" || it.key() == "minecraft:ender_chest"
                        || it.key() == "minecraft:stonecutter_block" || it.key() == "minecraft:trapped_chest") {
                        if (data == 2) data = 2;
                        else if (data == 3) data = 0;
                        else if (data == 4) data = 1;
                        else if (data == 5) data = 3;
                        else continue;
                    }
                    if (oldinfo766["data"] == data) {
                        RunTimeIdTable[766][info["hashruntimeid"]]          = oldinfo766["hashruntimeid"];
                        RunTimeIdTableOld[766][oldinfo766["hashruntimeid"]] = info["hashruntimeid"];
                    }
                }
            }
            {
                auto blocktypename = it.key();
                if (blocktypename == "minecraft:planks") { blocktypename = "minecraft:oak_planks"; }

                auto oldblock776 = json776[blocktypename];
                for (auto oldinfo776 : oldblock776) {
                    int data = info["data"];
                    if (blocktypename == "minecraft:chest" || blocktypename == "minecraft:ender_chest"
                        || blocktypename == "minecraft:stonecutter_block"
                        || blocktypename == "minecraft:trapped_chest") {
                        if (data == 2) data = 2;      
                        else if (data == 3) data = 0; 
                        else if (data == 4) data = 1; 
                        else if (data == 5) data = 3; 
                        else continue;
                    }

                    if (oldinfo776["data"] == data) {
                        RunTimeIdTable[776][info["hashruntimeid"]]          = oldinfo776["hashruntimeid"];
                        RunTimeIdTableOld[776][oldinfo776["hashruntimeid"]] = info["hashruntimeid"];
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
    HookPriority::Normal,
    "?initFromBlockDefinitions@BlockPalette@@QEAAXXZ",
    void,
    void* a1
) {
    origin(a1);
    ll::getLogger().info("Glacie is initializing, please wait!");
    auto blockPalette = new GlobalBlockPalette();
    blockPalette->init();
    GlobalBlockP = blockPalette;
    ll::getLogger().info("Glacie initialized successfully!");
}
