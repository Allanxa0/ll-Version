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
    std::vector<int> protocols = {859, 860, 898, 924};

    for (int protocol : protocols) {
        auto jsonItems = initMap("ITEM_DATA_" + std::to_string(protocol));
        auto jsonContent = initMap("CREATIVE_CONTENT_" + std::to_string(protocol));
        
        CREATIVE_CONTENTS[protocol] = jsonContent;
        
        std::vector<ItemData> vectorItems;
        for (auto const& i : jsonItems) {
            vectorItems.emplace_back(
                i["Name"].get<std::string>(),
                i["Id"].get<short>(),
                i["IsComponentBased"].get<bool>()
            );
        }
        ItemDataTable[protocol] = std::move(vectorItems);
    }
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

LL_TYPE_INSTANCE_HOOK(
    DeferredDescriptorInitHook,
    ll::memory::HookPriority::Normal,
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

LL_TYPE_INSTANCE_HOOK(
    DeferredDescriptorResolveHook,
    ll::memory::HookPriority::Normal,
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
