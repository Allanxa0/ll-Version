#include "ll/api/memory/Hook.h"
#include "mc/network/packet/InventoryContentPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/world/item/NetworkItemStackDescriptor.h"

extern void Serialize_630(NetworkItemStackDescriptor const& NetItem, BinaryStream& bs);

LL_AUTO_TYPE_INSTANCE_HOOK(
    InventoryContentPacketWrite,
    ll::memory::HookPriority::Normal,
    InventoryContentPacket,
    "?write@InventoryContentPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion >= 630) {
        bs.writeUnsignedVarInt(static_cast<uchar>(this->mInventoryId));
        bs.writeUnsignedVarInt(static_cast<uint>(this->mSlots.size()));
        for (auto& i : this->mSlots) {
            i.mIncludeNetIds = true;
            Serialize_630(i, bs);
        }
    } else {
        origin(bs);
    }
}
