#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Symbol.h"
#include "mc/network/packet/CraftingDataPacket.h"
#include "mc/network/packet/CraftingDataEntry.h"
#include "mc/network/packet/PotionMixDataEntry.h"
#include "mc/network/packet/ContainerMixDataEntry.h"
#include "mc/network/packet/MaterialReducerDataEntry.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/ItemDescriptor.h"
#include "mc/world/item/crafting/Recipe.h"
#include "glacie/core/item/GlobalItemData.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

extern uint64_t GlobalGuid;
extern std::unordered_map<uint64_t, int> PlayerGuidMap;
extern GlobalItemData* GlobalItemDataP;

using namespace ll::memory_literals;

static std::unordered_map<int, std::unordered_set<int16_t>> sValidItemsPerProtocol;
static bool sItemSetsBuilt = false;

static void ensureItemSetsBuilt() {
    if (sItemSetsBuilt || !GlobalItemDataP) return;
    sItemSetsBuilt = true;
    for (int protocol : {859, 860, 898, 924}) {
        for (auto const& item : GlobalItemDataP->getItemData(protocol)) {
            sValidItemsPerProtocol[protocol].insert(item.mId);
        }
    }
}

static bool isItemAvailable(int16_t id, int protocol) {
    if (id == 0) return true;
    auto it = sValidItemsPerProtocol.find(protocol);
    if (it == sValidItemsPerProtocol.end()) return true;
    return it->second.count(id) > 0;
}

static bool isEntryAvailable(CraftingDataEntry const& entry, int protocol) {
    auto const& recipePtr = entry.mRecipe.get();
    if (recipePtr) {
        for (auto const& result : recipePtr->getResultItems()) {
            auto const* item = result.getItem();
            if (item && !isItemAvailable(item->mId, protocol)) return false;
        }
        for (auto const& ing : recipePtr->mMyIngredients.get()) {
            if (!ing.isNull()) {
                auto const* item = ing.getItem();
                if (item && !isItemAvailable(item->mId, protocol)) return false;
            }
        }
    } else {
        auto const& result = entry.mItemResult.get();
        if (!isItemAvailable(result.getId(), protocol)) return false;
    }
    return true;
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    CraftingDataPacketWrite,
    ll::memory::HookPriority::Normal,
    CraftingDataPacket,
    "?write@CraftingDataPacket@@UEBAXAEAVBinaryStream@@@Z"_sym,
    void,
    BinaryStream& bs
) {
    int clientProtocol = 924;
    if (GlobalGuid != 0 && PlayerGuidMap.count(GlobalGuid)) {
        clientProtocol = PlayerGuidMap[GlobalGuid];
    }

    if (clientProtocol >= 924) {
        origin(bs);
        return;
    }

    ensureItemSetsBuilt();

    CraftingDataPacket filtered;
    filtered.mClearRecipes.get()           = this->mClearRecipes.get();
    filtered.mPotionMixEntries.get()       = this->mPotionMixEntries.get();
    filtered.mContainerMixEntries.get()    = this->mContainerMixEntries.get();
    filtered.mMaterialReducerEntries.get() = this->mMaterialReducerEntries.get();

    auto& dest = filtered.mCraftingEntries.get();
    dest.reserve(this->mCraftingEntries.get().size());

    for (auto const& entry : this->mCraftingEntries.get()) {
        if (isEntryAvailable(entry, clientProtocol)) {
            dest.push_back(entry);
        }
    }

    ((&filtered)->*&CraftingDataPacket::write)(bs);
}