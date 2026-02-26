#include "ll/api/memory/Hook.h"
#include "mc/network/packet/StartGamePacket.h"
#include "mc/network/packet/AddItemActorPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/common/SharedConstants.h"
#include <unordered_set>

extern GlobalItemData* GlobalItemDataP;

std::unordered_set<std::string> NewItem_630 = {
    "minecraft:andesite",
    "minecraft:diorite",
    "minecraft:granite",
    "minecraft:polished_andesite",
    "minecraft:polished_diorite",
    "minecraft:polished_granite",
    "minecraft:stone",
    "minecraft:oak_planks",
    "minecraft:birch_planks",
    "minecraft:jungle_planks",
    "minecraft:acacia_planks",
    "minecraft:dark_oak_planks"
};

LL_AUTO_TYPE_INSTANCE_HOOK(
    StartGamePacketWrite,
    ll::memory::HookPriority::Normal,
    StartGamePacket,
    "?write@StartGamePacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    this->mServerBlockTypeRegistryChecksum = 0;
    
    if (this->mProtocolVersion == 630) {
        for (auto& item : this->mItemData) {
            if (item.mName.getString() == "minecraft:planks") {
                item.mName = HashedString("minecraft:oak_planks");
                item.mId = 5;
            }
        }
        
        for (auto const& item : GlobalItemDataP->getItemData(630)) {
            if (NewItem_630.find(item.mName.getString()) != NewItem_630.end()) {
                this->mItemData.emplace_back(HashedString(item.mName.getString()), item.mId, item.mIsComponentBased);
            }
        }
    }
    origin(bs);
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    AddItemActorPacketWrite,
    ll::memory::HookPriority::Normal,
    AddItemActorPacket,
    "?write@AddItemActorPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion != SharedConstants::NetworkProtocolVersion) {
        this->mItem.mBlockRuntimeId =
            GlobalBlockP->getBlockRuntimeIdFromMain(this->mProtocolVersion, this->mItem.mBlockRuntimeId);
    }
    origin(bs);
}
