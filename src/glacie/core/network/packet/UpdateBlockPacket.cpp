#include "ll/api/memory/Hook.h"
#include "mc/network/packet/UpdateBlockPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/common/SharedConstants.h"

LL_AUTO_TYPE_INSTANCE_HOOK(
    UpdateBlockPacketWrite,
    ll::memory::HookPriority::Normal,
    UpdateBlockPacket,
    "?write@UpdateBlockPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion != SharedConstants::NetworkProtocolVersion) {
        bs.writeVarInt(this->mPos.x);
        bs.writeUnsignedVarInt(this->mPos.y);
        bs.writeVarInt(this->mPos.z);
        
        
        uint compatibleRuntimeId = GlobalBlockP->getBlockRuntimeIdFromMain(this->mProtocolVersion, this->mRuntimeId);
        bs.writeUnsignedVarInt(compatibleRuntimeId);
        
        bs.writeUnsignedVarInt(static_cast<uint>(this->mUpdateFlags));
        bs.writeUnsignedVarInt(static_cast<uint>(this->mLayer));
    } else {
        origin(bs);
    }
}
