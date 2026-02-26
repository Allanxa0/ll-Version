#include "ll/api/memory/Hook.h"
#include "mc/network/packet/UpdateSubChunkBlocksPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/common/SharedConstants.h"

LL_AUTO_TYPE_INSTANCE_HOOK(
    UpdateSubBlockPacketWrite,
    ll::memory::HookPriority::Normal,
    UpdateSubChunkBlocksPacket,
    "?write@UpdateSubChunkBlocksPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion != SharedConstants::NetworkProtocolVersion) {
        
        bs.writeVarInt(this->mSubChunkBlockPosition.x);
        bs.writeUnsignedVarInt(this->mSubChunkBlockPosition.y);
        bs.writeVarInt(this->mSubChunkBlockPosition.z);

        
        bs.writeUnsignedVarInt(static_cast<uint>(this->mBlocksChanged.mStandards.size()));
        for (auto const& info : this->mBlocksChanged.mStandards) {
            bs.writeVarInt(info.mPos.x);
            bs.writeUnsignedVarInt(info.mPos.y);
            bs.writeVarInt(info.mPos.z);
            
            uint compatibleId = GlobalBlockP->getBlockRuntimeIdFromMain(this->mProtocolVersion, info.mRuntimeId);
            bs.writeUnsignedVarInt(compatibleId);
            
            bs.writeUnsignedVarInt(info.mUpdateFlags);
            bs.writeVarInt64(info.mSyncMessage.mEntityUniqueID.id);
            bs.writeUnsignedVarInt(static_cast<uint>(info.mSyncMessage.mMessage));
        }

        
        bs.writeUnsignedVarInt(static_cast<uint>(this->mBlocksChanged.mExtras.size()));
        for (auto const& info : this->mBlocksChanged.mExtras) {
            bs.writeVarInt(info.mPos.x);
            bs.writeUnsignedVarInt(info.mPos.y);
            bs.writeVarInt(info.mPos.z);
            
            uint compatibleId = GlobalBlockP->getBlockRuntimeIdFromMain(this->mProtocolVersion, info.mRuntimeId);
            bs.writeUnsignedVarInt(compatibleId);
            
            bs.writeUnsignedVarInt(info.mUpdateFlags);
            bs.writeVarInt64(info.mSyncMessage.mEntityUniqueID.id);
            bs.writeUnsignedVarInt(static_cast<uint>(info.mSyncMessage.mMessage));
        }
    } else {
        origin(bs);
    }
}
