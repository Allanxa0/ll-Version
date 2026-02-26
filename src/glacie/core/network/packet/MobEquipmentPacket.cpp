#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/network/packet/MobEquipmentPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/world/item/registry/ItemRegistryManager.h"
#include "mc/world/item/registry/ItemRegistry.h"
#include "mc/world/item/NetworkItemStackDescriptor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

inline void Serialize_630(NetworkItemStackDescriptor const& NetItem, BinaryStream& bs) {
    auto id = NetItem.getId();
    auto StackSize = NetItem.getStackSize();
    auto itemRegistry = ItemRegistryManager::getItemRegistry()._lockRegistry();
    auto item = itemRegistry->getItem(id);
    short auxValue = 0;

    auto idaux = NetItem.getIdAux();
    if (!item.expired() && NetItem.isValid(true)) {
        switch (idaux) {
            case 327681: bs.writeVarInt(-739); bs.writeUnsignedShort(StackSize); bs.writeUnsignedVarInt(0); break;
            case 327682: bs.writeVarInt(-740); bs.writeUnsignedShort(StackSize); bs.writeUnsignedVarInt(0); break;
            case 327683: bs.writeVarInt(-741); bs.writeUnsignedShort(StackSize); bs.writeUnsignedVarInt(0); break;
            case 327684: bs.writeVarInt(-742); bs.writeUnsignedShort(StackSize); bs.writeUnsignedVarInt(0); break;
            case 327685: bs.writeVarInt(-743); bs.writeUnsignedShort(StackSize); bs.writeUnsignedVarInt(0); break;
            case 65537:  bs.writeVarInt(-590); bs.writeUnsignedShort(StackSize); bs.writeUnsignedVarInt(0); break;
            case 65539:  bs.writeVarInt(-592); bs.writeUnsignedShort(StackSize); bs.writeUnsignedVarInt(0); break;
            case 65541:  bs.writeVarInt(-594); bs.writeUnsignedShort(StackSize); bs.writeUnsignedVarInt(0); break;
            case 65538:  bs.writeVarInt(-591); bs.writeUnsignedShort(StackSize); bs.writeUnsignedVarInt(0); break;
            case 65540:  bs.writeVarInt(-593); bs.writeUnsignedShort(StackSize); bs.writeUnsignedVarInt(0); break;
            case 65542:  bs.writeVarInt(-595); bs.writeUnsignedShort(StackSize); bs.writeUnsignedVarInt(0); break;
            default: {
                bs.writeVarInt(id);
                bs.writeUnsignedShort(StackSize);
                if (!NetItem.getBlock()) { auxValue = NetItem.getAuxValue(); }
                bs.writeUnsignedVarInt(auxValue);
            }
        }

        bs.writeBool(NetItem.mIncludeNetIds);
        if (NetItem.mIncludeNetIds) {
            NetItem.mNetIdVariant.serialize(bs);
        }
        bs.writeVarInt(GlobalBlockP->getBlockRuntimeIdFromMain(630, NetItem.mBlockRuntimeId));
        bs.writeString(NetItem.mUserDataBuffer);
    } else {
        bs.writeVarInt(0);
    }
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    MobEquipmentPacketWrite,
    ll::memory::HookPriority::Normal,
    MobEquipmentPacket,
    "?write@MobEquipmentPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion == 630) {
        bs.writeUnsignedVarInt64(this->mRuntimeId.id);
        Serialize_630(this->mItem, bs);
        bs.writeUnsignedChar(this->mSlotByte);
        bs.writeUnsignedChar(this->mSelectedSlotByte);
        bs.writeUnsignedChar(this->mContainerIdByte);
    } else {
        origin(bs);
    }
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    MobEquipmentPacketHandle,
    ll::memory::HookPriority::Normal,
    ServerNetworkHandler,
    "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVMobEquipmentPacket@@@Z",
    void,
    NetworkIdentifier const& a1,
    MobEquipmentPacket const& a2
) {
    if (a2.mProtocolVersion == 630) {
        auto* level = ll::service::getLevel().get();
        if (level) {
            auto* ac = level->getRuntimeEntity(a2.mRuntimeId);
            if (ac) {
                const_cast<MobEquipmentPacket&>(a2).mItem.mBlockRuntimeId = 
                    GlobalBlockP->getBlockRuntimeIdFromOther(630, a2.mItem.mBlockRuntimeId);
                
                if (!ac->isPlayer()) { 
                    origin(a1, a2);
                    return;
                }
                
                auto* pl = static_cast<Player*>(ac);
                auto const& item = pl->getInventory().getItem(a2.mSlot);
                auto pkt = MobEquipmentPacket(a2.mRuntimeId, item, a2.mSlot, a2.mSelectedSlot, a2.mContainerId);
                origin(a1, pkt);
                return;
            }
        }
    }
    origin(a1, a2);
}
