#include "ll/api/memory/Hook.h"
#include "mc/network/packet/InventorySlotPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/world/item/NetworkItemStackDescriptor.h"

extern void Serialize_630(NetworkItemStackDescriptor const& NetItem, BinaryStream& bs);

LL_AUTO_TYPE_INSTANCE_HOOK(
    InventorySlotPacketWrite,
    ll::memory::HookPriority::Normal,
    InventorySlotPacket,
    "?write@InventorySlotPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion >= 630) {
        bs.writeUnsignedVarInt(static_cast<uchar>(this->mInventoryId));
        bs.writeUnsignedVarInt(this->mSlot);
        
        this->mItem.tryGetServerNetId();
        
        if (this->mItem.isValid(true) && this->mItem.getItem() && !this->mItem.isNull() && this->mItem.getStackSize() > 0) {
            this->mItem.mIncludeNetIds = true;
        }
        
        Serialize_630(this->mItem, bs);
    } else {
        origin(bs);
    }
}
