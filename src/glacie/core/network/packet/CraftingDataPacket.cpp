#include "ll/api/memory/Hook.h"
#include "mc/network/packet/CraftingDataPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/world/item/ItemInstance.h"
#include "mc/world/item/NetworkItemStackDescriptor.h"
#include "mc/world/item/registry/ItemRegistryManager.h"
#include "mc/world/item/registry/ItemRegistry.h"
#include "mc/world/item/ItemDescriptor.h"
#include "mc/world/item/crafting/Recipe.h"

void InsSerialize_630(ItemInstance const& itemInstance, BinaryStream& stream) {
    NetworkItemInstanceDescriptor networkitem(itemInstance);
    int16 id = networkitem.getId();
    int16 stackSize = networkitem.getStackSize();
    auto itemRegistry = ItemRegistryManager::getItemRegistry()._lockRegistry();
    auto item = itemRegistry->getItem(id);
    short auxValue = 0;
    
    if (!item.expired() && networkitem.isValid(true)) {
        if (!networkitem.getBlock()) { auxValue = networkitem.getAuxValue(); }
        switch (networkitem.getIdAux()) {
            case 327681: stream.writeVarInt(-739); stream.writeUnsignedShort(stackSize); stream.writeUnsignedVarInt(0); break;
            case 327682: stream.writeVarInt(-740); stream.writeUnsignedShort(stackSize); stream.writeUnsignedVarInt(0); break;
            case 327683: stream.writeVarInt(-741); stream.writeUnsignedShort(stackSize); stream.writeUnsignedVarInt(0); break;
            case 327684: stream.writeVarInt(-742); stream.writeUnsignedShort(stackSize); stream.writeUnsignedVarInt(0); break;
            case 327685: stream.writeVarInt(-743); stream.writeUnsignedShort(stackSize); stream.writeUnsignedVarInt(0); break;
            case 65537:  stream.writeVarInt(-590); stream.writeUnsignedShort(stackSize); stream.writeUnsignedVarInt(0); break;
            case 65539:  stream.writeVarInt(-592); stream.writeUnsignedShort(stackSize); stream.writeUnsignedVarInt(0); break;
            case 65541:  stream.writeVarInt(-594); stream.writeUnsignedShort(stackSize); stream.writeUnsignedVarInt(0); break;
            case 65538:  stream.writeVarInt(-591); stream.writeUnsignedShort(stackSize); stream.writeUnsignedVarInt(0); break;
            case 65540:  stream.writeVarInt(-593); stream.writeUnsignedShort(stackSize); stream.writeUnsignedVarInt(0); break;
            case 65542:  stream.writeVarInt(-595); stream.writeUnsignedShort(stackSize); stream.writeUnsignedVarInt(0); break;
            default:     stream.writeVarInt(id);   stream.writeUnsignedShort(stackSize); stream.writeUnsignedVarInt(auxValue); break;
        }
        stream.writeVarInt(GlobalBlockP->getBlockRuntimeIdFromMain(630, networkitem.mBlockRuntimeId));
        stream.writeString(networkitem.mUserDataBuffer);
    } else {
        stream.writeVarInt(0);
    }
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    CraftingDataPacketWrite,
    ll::memory::HookPriority::Normal,
    CraftingDataPacket,
    "?write@CraftingDataPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion >= 630) {
        bs.writeUnsignedVarInt(static_cast<uint>(this->mCraftingEntries.size()));
        for (auto& recipe : this->mCraftingEntries) {
            int type = static_cast<int>(recipe.mEntryType);
            if (type == 0 || type == 1) {
                bs.writeVarInt(type);
                bs.writeString(recipe.mRecipe->mRecipeId);
                
                if (type == 1) {
                    bs.writeVarInt(recipe.mRecipe->mWidth);
                    bs.writeVarInt(recipe.mRecipe->mHeight);
                } else {
                    bs.writeUnsignedVarInt(static_cast<uint>(recipe.mRecipe->mMyIngredients.size()));
                }

                auto writeIngredient = [&](ItemDescriptor& ingredient) {
                    ingredient._resolve();
                    if (!ingredient.isNull()) {
                        bs.writeUnsignedChar(static_cast<uchar>(ingredient.mImpl->getType()));
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
                                bs.writeUnsignedShort(finalId);
                                bs.writeUnsignedShort(ingredient.mImpl->getItem().mAuxValue);
                            } else bs.writeUnsignedShort(0);
                        } else ingredient.mImpl->serialize(bs);
                    } else bs.writeUnsignedChar(0);
                    bs.writeVarInt(ingredient.mStackSize);
                };

                if (type == 1) {
                    for (int z = 0; z < recipe.mRecipe->mHeight; ++z)
                        for (int x = 0; x < recipe.mRecipe->mWidth; ++x)
                            writeIngredient(recipe.mRecipe->getIngredient(x, z));
                } else {
                    for (auto& ing : recipe.mRecipe->mMyIngredients) writeIngredient(ing);
                }

                bs.writeUnsignedVarInt(static_cast<uint>(recipe.mRecipe->getResultItem().size()));
                for (auto const& result : recipe.mRecipe->getResultItem()) InsSerialize_630(result, bs);
                
                bs.writeUnsignedInt64(recipe.mRecipe->mMyId.a);
                bs.writeUnsignedInt64(recipe.mRecipe->mMyId.b);
                bs.writeString(recipe.mRecipe->getTag().getString());
                bs.writeVarInt(recipe.mRecipe->mPriority);
                bs.writeUnsignedVarInt(recipe.mRecipe->mRecipeNetId.mRawId);
            } else {
                recipe.write(bs);
            }
        }
        
        bs.writeUnsignedVarInt(static_cast<uint>(this->mPotionMixEntries.size()));
        for (auto const& entry : mPotionMixEntries) {
            bs.writeVarInt(entry.fromItemId); bs.writeVarInt(entry.fromItemAux);
            bs.writeVarInt(entry.reagentItemId); bs.writeVarInt(entry.reagentItemAux);
            bs.writeVarInt(entry.toItemId); bs.writeVarInt(entry.toItemAux);
        }

        bs.writeUnsignedVarInt(static_cast<uint>(this->mContainerMixEntries.size()));
        for (auto const& entry : mContainerMixEntries) {
            bs.writeVarInt(entry.fromItemId); bs.writeVarInt(entry.reagentItemId); bs.writeVarInt(entry.toItemId);
        }

        bs.writeUnsignedVarInt(static_cast<uint>(this->mMaterialReducerEntries.size()));
        for (auto const& entry : mMaterialReducerEntries) {
            bs.writeVarInt(entry.fromItemKey);
            bs.writeUnsignedVarInt(static_cast<uint>(entry.toItemIdsAndCounts.size()));
            for (auto const& ita : entry.toItemIdsAndCounts) {
                bs.writeVarInt(ita.itemId); bs.writeVarInt(ita.itemCount);
            }
        }
        bs.writeBool(this->mClearRecipes);
    } else {
        origin(bs);
    }
}
