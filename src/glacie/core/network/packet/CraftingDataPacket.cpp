#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Symbol.h"
#include "mc/network/packet/CraftingDataPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/world/item/ItemInstance.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/NetworkItemStackDescriptor.h"
#include "mc/world/item/registry/ItemRegistryManager.h"
#include "mc/world/item/registry/ItemRegistry.h"
#include "mc/world/item/ItemDescriptor.h"
#include "mc/world/item/crafting/Recipe.h"
#include "mc/world/item/crafting/RecipeIngredient.h"
#include <unordered_map>

extern GlobalBlockPalette* GlobalBlockP;
extern uint64 GlobalGuid;
extern std::unordered_map<uint64, int> PlayerGuidMap;

using namespace ll::memory_literals;

template <typename T>
void InsSerialize_630(T const& itemInstance, BinaryStream& stream) {
    NetworkItemStackDescriptor networkitem(itemInstance);
    int16 id = networkitem.getId();
    int16 stackSize = networkitem.getStackSize();
    auto itemRegistry = ItemRegistryManager::getItemRegistry()._lockRegistry();
    auto item = itemRegistry->getItem(id);
    short auxValue = 0;
    
    if (!item.expired() && networkitem.isValid(true)) {
        if (!networkitem.getBlock()) { auxValue = networkitem.getAuxValue(); }
        switch (networkitem.getIdAux()) {
            case 327681: stream.writeVarInt(-739, nullptr, nullptr); stream.writeUnsignedShort(stackSize, nullptr, nullptr); stream.writeUnsignedVarInt(0, nullptr, nullptr); break;
            case 327682: stream.writeVarInt(-740, nullptr, nullptr); stream.writeUnsignedShort(stackSize, nullptr, nullptr); stream.writeUnsignedVarInt(0, nullptr, nullptr); break;
            case 327683: stream.writeVarInt(-741, nullptr, nullptr); stream.writeUnsignedShort(stackSize, nullptr, nullptr); stream.writeUnsignedVarInt(0, nullptr, nullptr); break;
            case 327684: stream.writeVarInt(-742, nullptr, nullptr); stream.writeUnsignedShort(stackSize, nullptr, nullptr); stream.writeUnsignedVarInt(0, nullptr, nullptr); break;
            case 327685: stream.writeVarInt(-743, nullptr, nullptr); stream.writeUnsignedShort(stackSize, nullptr, nullptr); stream.writeUnsignedVarInt(0, nullptr, nullptr); break;
            case 65537:  stream.writeVarInt(-590, nullptr, nullptr); stream.writeUnsignedShort(stackSize, nullptr, nullptr); stream.writeUnsignedVarInt(0, nullptr, nullptr); break;
            case 65539:  stream.writeVarInt(-592, nullptr, nullptr); stream.writeUnsignedShort(stackSize, nullptr, nullptr); stream.writeUnsignedVarInt(0, nullptr, nullptr); break;
            case 65541:  stream.writeVarInt(-594, nullptr, nullptr); stream.writeUnsignedShort(stackSize, nullptr, nullptr); stream.writeUnsignedVarInt(0, nullptr, nullptr); break;
            case 65538:  stream.writeVarInt(-591, nullptr, nullptr); stream.writeUnsignedShort(stackSize, nullptr, nullptr); stream.writeUnsignedVarInt(0, nullptr, nullptr); break;
            case 65540:  stream.writeVarInt(-593, nullptr, nullptr); stream.writeUnsignedShort(stackSize, nullptr, nullptr); stream.writeUnsignedVarInt(0, nullptr, nullptr); break;
            case 65542:  stream.writeVarInt(-595, nullptr, nullptr); stream.writeUnsignedShort(stackSize, nullptr, nullptr); stream.writeUnsignedVarInt(0, nullptr, nullptr); break;
            default:     stream.writeVarInt(id, nullptr, nullptr);   stream.writeUnsignedShort(stackSize, nullptr, nullptr); stream.writeUnsignedVarInt(auxValue, nullptr, nullptr); break;
        }
        stream.writeVarInt(GlobalBlockP->getBlockRuntimeIdFromMain(630, networkitem.mBlockRuntimeId), nullptr, nullptr);
        stream.writeString(networkitem.mUserDataBuffer, nullptr, nullptr);
    } else {
        stream.writeVarInt(0, nullptr, nullptr);
    }
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    CraftingDataPacketWrite,
    ll::memory::HookPriority::Normal,
    CraftingDataPacket,
    "?write@CraftingDataPacket@@UEBAXAEAVBinaryStream@@@Z"_sym,
    void,
    BinaryStream& bs
) {
    int clientProtocol = 898; 
    if (GlobalGuid != 0 && PlayerGuidMap.count(GlobalGuid)) {
        clientProtocol = PlayerGuidMap[GlobalGuid];
    }

    if (clientProtocol >= 630 && clientProtocol < 859) {
        auto const& craftingEntries = this->mCraftingEntries.get();
        bs.writeUnsignedVarInt(static_cast<uint>(craftingEntries.size()), nullptr, nullptr);
        for (auto& recipe : craftingEntries) {
            int type = static_cast<int>(recipe.mEntryType);
            if (type == 0 || type == 1) {
                bs.writeVarInt(type, nullptr, nullptr);
                bs.writeString(recipe.mRecipe->mRecipeId, nullptr, nullptr);
                
                if (type == 1) {
                    bs.writeVarInt(recipe.mRecipe->mWidth, nullptr, nullptr);
                    bs.writeVarInt(recipe.mRecipe->mHeight, nullptr, nullptr);
                } else {
                    bs.writeUnsignedVarInt(static_cast<uint>(recipe.mRecipe->mMyIngredients.size()), nullptr, nullptr);
                }

                auto writeIngredient = [&](auto const& ingredient) {
                    if (!ingredient.isNull()) {
                        bs.writeByte(static_cast<uchar>(ingredient.mImpl->getType()), nullptr, nullptr);
                        if (ingredient.mImpl->getType() == ItemDescriptor::InternalType::Default) {
                            auto dItem = ingredient.mImpl->getItem().mItem;
                            if (dItem) {
                                int16 finalId = dItem->getId();
                                switch (ingredient.getIdAux()) {
                                    case 327681: finalId = -739; break;
                                    case 327682: finalId = -740; break;
                                    case 327683: finalId = -741; break;
                                    case 327684: finalId = -742; break;
                                    case 327685: finalId = -743; break;
                                    case 65537:  finalId = -590; break;
                                    case 65539:  finalId = -592; break;
                                    case 65541:  finalId = -594; break;
                                    case 65538:  finalId = -591; break;
                                    case 65540:  finalId = -593; break;
                                    case 65542:  finalId = -595; break;
                                }
                                bs.writeUnsignedShort(finalId, nullptr, nullptr);
                                bs.writeUnsignedShort(ingredient.mImpl->getItem().mAuxValue, nullptr, nullptr);
                            } else bs.writeUnsignedShort(0, nullptr, nullptr);
                        } else ingredient.mImpl->serialize(bs);
                    } else bs.writeByte(0, nullptr, nullptr);
                    bs.writeVarInt(1, nullptr, nullptr); 
                };

                if (type == 1) {
                    for (int z = 0; z < recipe.mRecipe->mHeight; ++z)
                        for (int x = 0; x < recipe.mRecipe->mWidth; ++x)
                            writeIngredient(recipe.mRecipe->getIngredient(x, z));
                } else {
                    for (auto const& ing : recipe.mRecipe->mMyIngredients) writeIngredient(ing);
                }

                auto const& results = recipe.mRecipe->getResultItem();
                bs.writeUnsignedVarInt(static_cast<uint>(results.size()), nullptr, nullptr);
                for (auto const& result : results) InsSerialize_630(result, bs);
                
                bs.writeUnsignedInt64(recipe.mRecipe->mMyId.a, nullptr, nullptr);
                bs.writeUnsignedInt64(recipe.mRecipe->mMyId.b, nullptr, nullptr);
                bs.writeString(recipe.mRecipe->getTag().getString(), nullptr, nullptr);
                bs.writeVarInt(recipe.mRecipe->mPriority, nullptr, nullptr);
                bs.writeUnsignedVarInt(recipe.mRecipe->mRecipeNetId.mRawId, nullptr, nullptr);
            } else {
                recipe.write(bs);
            }
        }
        
        auto const& potionEntries = this->mPotionMixEntries.get();
        bs.writeUnsignedVarInt(static_cast<uint>(potionEntries.size()), nullptr, nullptr);
        for (auto const& entry : potionEntries) {
            bs.writeVarInt(entry.fromItemId, nullptr, nullptr); bs.writeVarInt(entry.fromItemAux, nullptr, nullptr);
            bs.writeVarInt(entry.reagentItemId, nullptr, nullptr); bs.writeVarInt(entry.reagentItemAux, nullptr, nullptr);
            bs.writeVarInt(entry.toItemId, nullptr, nullptr); bs.writeVarInt(entry.toItemAux, nullptr, nullptr);
        }

        auto const& containerEntries = this->mContainerMixEntries.get();
        bs.writeUnsignedVarInt(static_cast<uint>(containerEntries.size()), nullptr, nullptr);
        for (auto const& entry : containerEntries) {
            bs.writeVarInt(entry.fromItemId, nullptr, nullptr); bs.writeVarInt(entry.reagentItemId, nullptr, nullptr); bs.writeVarInt(entry.toItemId, nullptr, nullptr);
        }

        auto const& materialEntries = this->mMaterialReducerEntries.get();
        bs.writeUnsignedVarInt(static_cast<uint>(materialEntries.size()), nullptr, nullptr);
        for (auto const& entry : materialEntries) {
            bs.writeVarInt(entry.fromItemKey, nullptr, nullptr);
            bs.writeUnsignedVarInt(static_cast<uint>(entry.toItemIdsAndCounts.size()), nullptr, nullptr);
            for (auto const& ita : entry.toItemIdsAndCounts) {
                bs.writeVarInt(ita.itemId, nullptr, nullptr); bs.writeVarInt(ita.itemCount, nullptr, nullptr);
            }
        }
        bs.writeBool(this->mClearRecipes.get(), nullptr, nullptr);
    } else {
        origin(bs);
    }
}
