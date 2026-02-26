#include "ll/api/memory/Hook.h"
#include "mc/network/packet/InventoryTransactionPacket.h"
#include "mc/world/ContainerID.h"
#include "mc/world/inventory/transaction/InventoryAction.h"
#include "mc/world/inventory/transaction/InventorySource.h"
#include "mc/world/inventory/transaction/InventoryTransaction.h"
#include "mc/world/inventory/transaction/ComplexInventoryTransaction.h"
#include "mc/world/inventory/transaction/ItemUseInventoryTransaction.h"
#include "mc/world/item/NetworkItemStackDescriptor.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/actor/player/Player.h"
#include "mc/common/SharedConstants.h"
#include <unordered_map>
#include <unordered_set>

extern std::unordered_map<uint64, int> PlayerGuidMap;

bool setValuesAndMatch(ItemStack* a1, ItemStack* a2) {
    if (!a1 || !a2) return false;
    auto AuxValue = a1->getAuxValue();
    a2->setAuxValue(AuxValue);
    if (a1->hasDamageValue()) {
        auto DamageValue = a1->getDamageValue();
        a2->setDamageValue(DamageValue);
    } else {
        a2->removeDamageValue();
    }
    return a1->matches(*a2);
}

std::unordered_set<std::string> TranslateItemMap = {
    "minecraft:chest",
    "minecraft:ender_chest",
    "minecraft:stonecutter_block",
    "minecraft:trapped_chest",
    "minecraft:birch_planks",
    "minecraft:jungle_planks",
    "andesite",
    "diorite",
    "granite",
    "polished_andesite",
    "polished_diorite",
    "polished_granite",
    "stone"
};

bool isTranslateItem(std::string const& item) {
    return TranslateItemMap.find(item) != TranslateItemMap.end();
}

extern void Serialize_630(NetworkItemStackDescriptor const& NetItem, BinaryStream& bs);

LL_AUTO_TYPE_INSTANCE_HOOK(
    InventoryTransactionPacketWrite,
    ll::memory::HookPriority::Normal,
    InventoryTransactionPacket,
    "?write@InventoryTransactionPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion >= 630) {
        int legacyRequestId = this->mLegacyRequestId.mRawId;
        bs.writeVarInt(legacyRequestId);
        
        if (legacyRequestId < -1 && (legacyRequestId & 1) == 0) {
            bs.writeUnsignedVarInt(static_cast<uint>(this->mLegacySetItemSlots.size()));
            for (auto const& slot : this->mLegacySetItemSlots) {
                bs.writeByte(static_cast<uchar>(slot.first));
                bs.writeUnsignedVarInt(static_cast<uint>(slot.second.size()));
                for (auto key : slot.second) { bs.writeByte(key); }
            }
        }

        if (this->mTransaction) {
            ComplexInventoryTransaction::Type transactionType = this->mTransaction->type;
            bs.writeUnsignedVarInt(static_cast<uint>(transactionType));
            
            uint totalActions = 0;
            for (auto const& actionGroup : this->mTransaction->data.actions) { 
                totalActions += static_cast<uint>(actionGroup.second.size()); 
            }
            
            bs.writeUnsignedVarInt(totalActions);
            
            for (auto& actionGroup : this->mTransaction->data.actions) {
                for (auto& transaction : actionGroup.second) {
                    int sourceType = static_cast<int>(transaction.mSource.mType);
                    
                    if (!this->mIsClientSide) {
                        if (transaction.mToItem.mValid && !transaction.mToItem.mItem.expired() && 
                            !transaction.mToItem.isNull() && transaction.mToItem.mCount > 0) {
                            
                            if ((sourceType != 3 && sourceType != 2) || transaction.mSlot != 0) {
                                transaction.mToItem.tryGetItemStackNetId();
                                transaction.mToItemDescriptor.mIncludeNetIds = true;
                            }
                        }
                    }

                    bs.writeUnsignedVarInt(sourceType);
                    switch (sourceType) {
                        case 0:
                        case 99999:
                            bs.writeVarInt(static_cast<int>(transaction.mSource.mContainerId));
                            break;
                        case 2:
                            bs.writeUnsignedVarInt(static_cast<uint>(transaction.mSource.mFlags));
                            break;
                        default:
                            break;
                    }
                    
                    bs.writeUnsignedVarInt(transaction.mSlot);
                    Serialize_630(transaction.mFromItemDescriptor, bs);
                    Serialize_630(transaction.mToItemDescriptor, bs);
                }
            }
            this->mTransaction->write(bs);
        }
    } else {
        origin(bs);
    }
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    ItemUseInventoryTransactionHandle,
    ll::memory::HookPriority::Normal,
    ItemUseInventoryTransaction,
    "?handle@ItemUseInventoryTransaction@@UEBA?AW4InventoryTransactionError@@AEAVPlayer@@_N@Z",
    ::InventoryTransactionError,
    Player& pl,
    bool a1
) {
    auto ver = PlayerGuidMap[pl.getNetworkIdentifier().mGuid.g];
    if (ver != 0 && ver != SharedConstants::NetworkProtocolVersion) {
        this->mTargetBlockId = GlobalBlockP->getBlockRuntimeIdFromOther(ver, this->mTargetBlockId);
    }
    return origin(pl, a1);
}
