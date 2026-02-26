#include "ll/api/memory/Hook.h"
#include "mc/network/packet/AddItemActorPacket.h"
#include "mc/world/item/NetworkItemStackDescriptor.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include <unordered_map>

extern uint64 GlobalGuid;
extern std::unordered_map<uint64, int> PlayerGuidMap;

LL_AUTO_TYPE_INSTANCE_HOOK(
    AddItemActorPacketWrite,
    ll::memory::HookPriority::Normal,
    AddItemActorPacket,
    &AddItemActorPacket::$write,
    void,
    BinaryStream& bs
) {
    int clientProtocol = 898; 
    if (GlobalGuid != 0 && PlayerGuidMap.count(GlobalGuid)) {
        clientProtocol = PlayerGuidMap[GlobalGuid];
    }

    if (clientProtocol >= 859) {
        bs.writeVarInt64(this->mId.get().rawID, nullptr, nullptr);
        bs.writeUnsignedVarInt64(this->mRuntimeId.get().rawID, nullptr, nullptr);
        
        bs.writeType(this->mItem.get());
        
        bs.writeType(this->mPos.get());
        bs.writeType(this->mVelocity.get());

        bs.writeType(this->mData.get());

        bs.writeBool(this->mIsFromFishing, nullptr, nullptr);
    } else {
        origin(bs);
    }
}
