#include "glacie/core/block/GlobalBlockPalette.h"
#include "glacie/core/internal/Logger.h"
#include "ll/api/memory/Hook.h"
#include <fstream>

GlobalBlockPalette* GlobalBlockP;

void GlobalBlockPalette::init() {
    auto json630 = initMap(L"BLOCK_INFO_1_20_50");
    auto json622 = initMap(L"BLOCK_INFO_1_20_40");
    auto json618 = initMap(L"BLOCK_INFO_1_20_30");

    for (nlohmann::json::iterator it = json618.begin(); it != json618.end(); ++it) {
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

                auto oldblock594 = json594[blocktypename];
                for (auto oldinfo594 : oldblock594) {
                    if (oldinfo594["data"] == data) {
                        RunTimeIdTable[594][info["hashruntimeid"]]          = oldinfo594["hashruntimeid"];
                        RunTimeIdTableOld[594][oldinfo594["hashruntimeid"]] = info["hashruntimeid"];
                    }
                }
            }

            {
                auto oldblock622 = json622[it.key()];
                for (auto oldinfo622 : oldblock622) {
                    int data = info["data"];
                    if (it.key() == "minecraft:chest" || it.key() == "minecraft:ender_chest"
                        || it.key() == "minecraft:stonecutter_block" || it.key() == "minecraft:trapped_chest") {
                        if (data == 2) data = 2;
                        else if (data == 3) data = 0;
                        else if (data == 4) data = 1;
                        else if (data == 5) data = 3;
                        else continue;
                    }
                    if (oldinfo622["data"] == data) {
                        RunTimeIdTable[622][info["hashruntimeid"]]          = oldinfo622["hashruntimeid"];
                        RunTimeIdTableOld[622][oldinfo622["hashruntimeid"]] = info["hashruntimeid"];
                    }
                }
            }
            {
                auto blocktypename = it.key();
                if (blocktypename == "minecraft:planks") { blocktypename = "minecraft:oak_planks"; }

                auto oldblock630 = json630[blocktypename];
                for (auto oldinfo630 : oldblock630) {
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

                    if (oldinfo630["data"] == data) {
                        RunTimeIdTable[630][info["hashruntimeid"]]          = oldinfo630["hashruntimeid"];
                        RunTimeIdTableOld[630][oldinfo630["hashruntimeid"]] = info["hashruntimeid"];
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
    Info("Glacie is initializing, please wait!");
    auto blockPalette = new GlobalBlockPalette();
    blockPalette->init();
    GlobalBlockP = blockPalette;
    Info("Glacie initialized successfully!");
}
