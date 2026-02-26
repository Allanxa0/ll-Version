#include "ll/api/memory/Hook.h"
#include "mc/network/packet/AddPlayerPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/world/item/NetworkItemStackDescriptor.h"
#include "mc/world/actor/SynchedActorDataEntityWrapper.h"
#include "mc/world/actor/SerializedAbilitiesData.h"
#include "mc/world/actor/ActorLink.h"
#include "mc/legacy/ActorRuntimeID.h"
#include <unordered_map>

extern uint64 GlobalGuid;
extern std::unordered_map<uint64, int> PlayerGuidMap;

LL_AUTO_TYPE_INSTANCE_HOOK(
    AddPlayerPacketWrite,
    ll::memory::HookPriority::Normal,
    AddPlayerPacket,
    &AddPlayerPacket::$write,
    void,
    BinaryStream& bs
) {
    int clientProtocol = 898; 
    if (GlobalGuid != 0 && PlayerGuidMap.count(GlobalGuid)) {
        clientProtocol = PlayerGuidMap[GlobalGuid];
    }

    if (clientProtocol == 859 || clientProtocol == 860 || clientProtocol == 924) {
        bs.writeUnsignedInt64(this->mUuid.get().a, nullptr, nullptr);
        bs.writeUnsignedInt64(this->mUuid.get().b, nullptr, nullptr);
        bs.writeString(this->mName.get(), nullptr, nullptr);
        
        bs.writeUnsignedVarInt64(this->mRuntimeId.get().rawID, nullptr, nullptr);
        bs.writeString(this->mPlatformOnlineId.get(), nullptr, nullptr);
        
        bs.writeType(this->mPos.get());
        bs.writeType(this->mVelocity.get());
        
        bs.writeFloat(this->mRot.get().x, nullptr, nullptr);
        bs.writeFloat(this->mRot.get().y, nullptr, nullptr);
        bs.writeFloat(this->mYHeadRot.get(), nullptr, nullptr);
        
        bs.writeType(this->mCarriedItem.get()); 
        
        bs.writeVarInt(static_cast<int>(this->mPlayerGameType.get()), nullptr, nullptr);
        
        bs.writeType(this->mUnpack.get());
        
        bs.writeType(this->mSynchedProperties.get());
        
        auto abilitiesData = SerializedAbilitiesData(this->mEntityId.get(), this->mAbilities.get());
        bs.writeType(abilitiesData);
        
        bs.writeUnsignedVarInt(static_cast<uint>(this->mLinks.get().size()), nullptr, nullptr);
        for (auto const& link : this->mLinks.get()) { 
            bs.writeType(link); 
        }
        
        bs.writeString(this->mDeviceId.get(), nullptr, nullptr);
        bs.writeUnsignedInt(static_cast<uint>(this->mBuildPlatform.get()), nullptr, nullptr);
    } else {
        origin(bs);
    }
}
