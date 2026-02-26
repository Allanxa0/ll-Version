#pragma once
#include "mc/_HeaderOutputPredefine.h"
#include "nlohmann/json.hpp"
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

struct ItemData {
    std::string mName;
    short mId;
    bool mIsComponentBased;
    ItemData(std::string name, short id, bool comp) : mName(std::move(name)), mId(id), mIsComponentBased(comp) {}
};

class GlobalItemData {
public:
    std::unordered_map<int, std::vector<ItemData>> ItemDataTable;
    std::unordered_map<int, nlohmann::json>        CREATIVE_CONTENTS;

    nlohmann::json initMap(std::string const& a1);
    void           init();

    std::vector<ItemData> getItemData(int protocol);
    nlohmann::json        getContent(int protocol);
};

extern GlobalItemData* GlobalItemDataP;
