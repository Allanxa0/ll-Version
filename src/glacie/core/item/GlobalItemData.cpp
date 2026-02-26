#include "glacie/core/item/GlobalItemData.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/item/DeferredDescriptor.h"
#include "mc/world/item/Item.h"
#include "mc/common/WeakPtr.h"
#include <fstream>
#include <string>
#include <vector>

GlobalItemData* GlobalItemDataP = nullptr;

void GlobalItemData::init() {
    auto json630 = initMap(L"ITEM_DATA_1_20_50");
    auto json630_Content = initMap(L"CREATIVE_CONTENT_1_20_50");
    
    CREATIVE_CONTENTS[630] = json630_Content;
    
    std::vector<ItemData> vector630;
    for (auto const& i : json630) {
        vector630.emplace_back(
            i["Name"].get<std::string>(),
            i["Id"].get<short>(),
            i["IsComponentBased"].get<bool>()
        );
    }
    ItemDataTable[630] = std::move(vector630);
}

std::vector<ItemData> GlobalItemData::getItemData(int protocol) {
    if (ItemDataTable.contains(protocol)) {
        return ItemDataTable[protocol];
    }
    return {};
}

nlohmann::json GlobalItemData::getContent(int protocol) {
    if (CREATIVE_CONTENTS.contains(protocol)) {
        return CREATIVE_CONTENTS[protocol];
    }
    return nlohmann::json::object();
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    DeferredDescriptorInitHook,
    HookPriority::Normal,
    DeferredDescriptor,
    &DeferredDescriptor::_initFromItem,
    std::unique_ptr<ItemDescriptor::BaseDescriptor>,
    WeakPtr<Item>&& item,
    short aux
) {
    if (!GlobalItemDataP) {
        GlobalItemDataP = new GlobalItemData();
        GlobalItemDataP->init();
    }
    return origin(std::move(item), aux);
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    DeferredDescriptorResolveHook,
    HookPriority::Normal,
    DeferredDescriptor,
    &DeferredDescriptor::$resolve,
    std::unique_ptr<ItemDescriptor::BaseDescriptor>
) {
    if (!GlobalItemDataP) {
        GlobalItemDataP = new GlobalItemData();
        GlobalItemDataP->init();
    }
    return origin();
}
