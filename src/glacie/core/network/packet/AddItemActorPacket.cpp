#include "ll/api/memory/Hook.h"
#include "mc/network/packet/AddItemActorPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "glacie/core/block/GlobalBlockPalette.h"

LL_TYPE_INSTANCE_HOOK(
    AddItemActorPacketWrite,
    ll::memory::HookPriority::Normal,
    AddItemActorPacket,
    &AddItemActorPacket::write,
    void,
    BinaryStream& bs
) {
    auto* packet = const_cast<AddItemActorPacket*>(this);
    uint oldRuntimeId = packet->mItem.mBlockRuntimeId;
    packet->mItem.mBlockRuntimeId = GlobalBlockP->getBlockRuntimeIdFromMain(packet->mProtocolVersion, oldRuntimeId);
    
    origin(bs);
    
    packet->mItem.mBlockRuntimeId = oldRuntimeId;
}
