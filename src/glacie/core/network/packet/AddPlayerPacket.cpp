#include "ll/api/memory/Hook.h"
#include "mc/network/packet/AddPlayerPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/world/item/NetworkItemStackDescriptor.h"
#include "mc/world/actor/SynchedActorDataEntityWrapper.h"
#include "mc/world/actor/SerializedAbilitiesData.h"

extern void Serialize_630(NetworkItemStackDescriptor NetItem, BinaryStream* bs);

LL_AUTO_TYPE_INSTANCE_HOOK(
    AddPlayerPacketWrite,
    ll::memory::HookPriority::Normal,
    AddPlayerPacket,
    "?write@AddPlayerPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion == 630) {
        bs.writeUnsignedInt64(this->mUuid.a);
        bs.writeUnsignedInt64(this->mUuid.b);
        bs.writeString(this->mName);
        bs.writeUnsignedVarInt64(this->mRuntimeId.id);
        bs.writeString(this->mPlatformOnlineId);
        bs.writeType(this->mPos);
        bs.writeType(this->mVelocity);
        bs.writeFloat(this->mRot.x);
        bs.writeFloat(this->mRot.y);
        bs.writeFloat(this->mYHeadRot);
        Serialize_630(this->mCarriedItem, &bs);
        bs.writeVarInt((int)this->mPlayerGameType);
        
        auto* mEntityData = reinterpret_cast<SynchedActorDataEntityWrapper*>(this->mEntityData);
        if (mEntityData) {
            auto packedData = mEntityData->packAll();
            bs.writeType(packedData);
        } else {
            bs.writeType(this->mUnpack);
        }
        
        bs.writeType(this->mSynchedProperties);
        auto abilitiesData = SerializedAbilitiesData(this->mEntityId, this->mAbilities);
        bs.writeType(abilitiesData);
        
        bs.writeUnsignedVarInt(static_cast<uint>(this->mLinks.size()));
        for (auto const& link : this->mLinks) { 
            bs.writeType(link); 
        }
        
        bs.writeString(this->mDeviceId);
        bs.writeUnsignedInt(static_cast<uint>(this->mBuildPlatform));
    } else {
        origin(bs);
    }
}
