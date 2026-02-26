#include "ll/api/memory/Hook.h"
#include "mc/network/packet/ResourcePacksInfoPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"

LL_AUTO_TYPE_INSTANCE_HOOK(
    ResourcePacksInfoPacketWrite,
    ll::memory::HookPriority::Normal,
    ResourcePacksInfoPacket,
    "?write@ResourcePacksInfoPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion >= 618) {
        origin(bs);
    } else {
        bs.writeBool(this->mData.mResourcePackRequired);
        bs.writeBool(this->mData.mHasScripts);
        bs.writeBool(this->mData.mForceServerPacksEnabled);
        
        bs.writeUnsignedShort(static_cast<unsigned short>(this->mData.mBehaviorPacks.size()));
        for (auto const& bp : this->mData.mBehaviorPacks) {
            bs.writeString(bp.mPackIdVersion.mId.asString());
            bs.writeString(bp.mPackIdVersion.mVersion.asString());
            bs.writeUnsignedInt64(bp.mPackSize);
            bs.writeString(bp.mContentKey);
            bs.writeString(bp.mSubpackName);
            bs.writeString(bp.mContentIdentity.asString());
            bs.writeBool(bp.mHasScripts);
        }
        
        bs.writeUnsignedShort(static_cast<unsigned short>(this->mData.mResourcePacks.size()));
        for (auto const& rp : this->mData.mResourcePacks) {
            bs.writeString(rp.mPackIdVersion.mId.asString());
            bs.writeString(rp.mPackIdVersion.mVersion.asString());
            bs.writeUnsignedInt64(rp.mPackSize);
            bs.writeString(rp.mContentKey);
            bs.writeString(rp.mSubpackName);
            bs.writeString(rp.mContentIdentity.asString());
            bs.writeBool(rp.mHasScripts);
            bs.writeBool(rp.mIsRayTracingCapable); 
        }
    }
}
