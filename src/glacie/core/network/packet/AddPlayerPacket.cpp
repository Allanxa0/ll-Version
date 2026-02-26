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
        
        bs.writeUnsignedInt64(this->mUuid.a, nullptr, nullptr);
        bs.writeUnsignedInt64(this->mUuid.b, nullptr, nullptr);
        bs.writeString(this->mName, nullptr, nullptr);
        
        bs.writeUnsignedVarInt64(this->mRuntimeId.rawID, nullptr, nullptr);
        bs.writeString(this->mPlatformOnlineId, nullptr, nullptr);
        
        bs.writeType(this->mPos);
        bs.writeType(this->mVelocity);
        
        bs.writeFloat(this->mRot.x, nullptr, nullptr);
        bs.writeFloat(this->mRot.y, nullptr, nullptr);
        bs.writeFloat(this->mYHeadRot, nullptr, nullptr);
        
        bs.writeType(this->mCarriedItem); 
        
        bs.writeVarInt(static_cast<int>(this->mPlayerGameType), nullptr, nullptr);
        
        bs.writeType(this->mUnpack);
        
        bs.writeType(this->mSynchedProperties);
        
        auto abilitiesData = SerializedAbilitiesData(this->mEntityId, this->mAbilities);
        bs.writeType(abilitiesData);
        
        bs.writeUnsignedVarInt(static_cast<uint>(this->mLinks.size()), nullptr, nullptr);
        for (auto const& link : this->mLinks) { 
            bs.writeType(link); 
        }
        
        bs.writeString(this->mDeviceId, nullptr, nullptr);
        bs.writeUnsignedInt(static_cast<uint>(this->mBuildPlatform), nullptr, nullptr);
        
    } else {
        origin(bs);
    }
}
