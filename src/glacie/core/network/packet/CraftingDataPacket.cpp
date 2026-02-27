#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Symbol.h"
#include "mc/network/packet/CraftingDataPacket.h"
#include "mc/network/packet/CraftingDataEntry.h"
#include "mc/network/packet/PotionMixDataEntry.h"
#include "mc/network/packet/ContainerMixDataEntry.h"
#include "mc/network/packet/MaterialReducerDataEntry.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/world/item/ItemInstance.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/registry/ItemRegistryManager.h"
#include "mc/world/item/ItemDescriptor.h"
#include "mc/world/item/crafting/Recipe.h"
#include <unordered_map>
#include <vector>

extern uint64_t GlobalGuid;
extern std::unordered_map<uint64_t, int> PlayerGuidMap;

std::unordered_map<int16_t, int> ItemMinProtocolMap = {
    
};

using namespace ll::memory_literals;

bool IsItemValid(int16_t id, int clientProtocol) {
    auto it = ItemMinProtocolMap.find(id);
    if (it != ItemMinProtocolMap.end()) {
        return clientProtocol >= it->second;
    }
    return true;
}

bool IsRecipeValid(CraftingDataEntry const& entry, int clientProtocol) {
    auto const& recipePtr = entry.mRecipe.get();
    if (recipePtr) {
        auto const& recipe = *recipePtr;
        for (auto const& ing : recipe.mMyIngredients.get()) {
            if (!ing.isNull()) {
                auto const* item = ing.getItem();
                if (item && !IsItemValid(item->mId, clientProtocol)) {
                    return false;
                }
            }
        }
        for (auto const& result : recipe.getResultItems()) {
            auto const* item = result.getItem();
            if (item && !IsItemValid(item->mId, clientProtocol)) {
                return false;
            }
        }
    } else {
        auto const& resultDescriptor = entry.mItemResult.get();
        if (resultDescriptor.mId != 0 && !IsItemValid(resultDescriptor.mId, clientProtocol)) {
            return false;
        }
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

    CraftingDataPacket tempPacket;
    tempPacket.mClearRecipes.get() = this->mClearRecipes.get();
    tempPacket.mPotionMixEntries.get() = this->mPotionMixEntries.get();
    tempPacket.mContainerMixEntries.get() = this->mContainerMixEntries.get();
    tempPacket.mMaterialReducerEntries.get() = this->mMaterialReducerEntries.get();

    auto const& oldEntries = this->mCraftingEntries.get();
    auto& newEntries = tempPacket.mCraftingEntries.get();
    newEntries.reserve(oldEntries.size());

    for (auto const& entry : oldEntries) {
        if (IsRecipeValid(entry, clientProtocol)) {
            newEntries.push_back(entry);
        }
    }

    ((&tempPacket)->*&CraftingDataPacket::write)(bs);
}
