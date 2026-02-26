#pragma once
#include "mc/_HeaderOutputPredefine.h"
#include "nlohmann/json.hpp"
#include <regex>
#include <string>
#include <unordered_map>

extern HMODULE t_hModule;

class GlobalBlockPalette {
public:
    std::unordered_map<int, std::unordered_map<uint, uint>> RunTimeIdTable;
    std::unordered_map<int, std::unordered_map<uint, uint>> RunTimeIdTableOld;

    nlohmann::json initMap(LPCWSTR a1);
    void           init();

    uint getBlockRuntimeIdFromMain(int protocol, uint oldruntimeid);
    uint getBlockRuntimeIdFromOther(int protocol, uint oldruntimeid);
};

extern GlobalBlockPalette* GlobalBlockP;
