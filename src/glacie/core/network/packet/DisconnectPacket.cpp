#include "ll/api/memory/Hook.h"
#include "mc/network/packet/DisconnectPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"

LL_AUTO_TYPE_INSTANCE_HOOK(
    DisconnectPacketWrite,
    ll::memory::HookPriority::Normal,
    DisconnectPacket,
    "?write@DisconnectPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion >= 622) {
        bs.writeVarInt(-1);
        bs.writeBool(this->mSkipMessage);
        bs.writeString(this->mMessage);
    } else {
        origin(bs);
    }
}
