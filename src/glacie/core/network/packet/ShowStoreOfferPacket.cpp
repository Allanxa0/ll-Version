#include "ll/api/memory/Hook.h"
#include "mc/network/packet/ShowStoreOfferPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"

LL_AUTO_TYPE_INSTANCE_HOOK(
    ShowStoreOfferPacketWrite,
    ll::memory::HookPriority::Normal,
    ShowStoreOfferPacket,
    "?write@ShowStoreOfferPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion > 622) {
        bs.writeString(this->mOfferId);
        bs.writeBool(this->mShowAll);
        bs.writeUnsignedVarInt(0);
    } else {
        origin(bs);
    }
}
