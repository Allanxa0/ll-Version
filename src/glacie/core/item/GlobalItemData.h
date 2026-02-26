#include "mc/_HeaderOutputPredefine.h"
#include "nlohmann/json.hpp"
#include <regex>
#include <string>

extern HMODULE t_hModule;
class GlobalItemData {
public:
    std::unordered_map<int, std::vector<struct ItemData>> ItemDataTable;
    std::unordered_map<int, nlohmann::json>               CREATIVE_CONTENTS;

    nlohmann::json initMap(LPCWSTR a1);
    void           init();

    std::vector<ItemData> getItemData(int protocol);
    nlohmann::json        getContent(int protocol);
};

extern GlobalItemData* GlobalItemDataP;