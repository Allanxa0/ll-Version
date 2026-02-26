#include "ll/api/memory/Hook.h"
#include "mc/network/packet/LevelEventPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/common/SharedConstants.h"

LL_AUTO_TYPE_INSTANCE_HOOK(
    LevelEventPacketWrite,
    ll::memory::HookPriority::Normal,
    LevelEventPacket,
    "?write@LevelEventPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion != SharedConstants::NetworkProtocolVersion) {
        if ((this->mEventId >= 3603 && this->mEventId <= 3608) || this->mEventId == 2001) {
            this->mData = GlobalBlockP->getBlockRuntimeIdFromMain(this->mProtocolVersion, this->mData);
        }
    }
    origin(bs);
}
