#include "ll/api/memory/Hook.h"
#include "mc/network/packet/CreativeContentPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/world/item/ItemInstance.h"
#include "mc/world/item/registry/ItemRegistryManager.h"
#include "mc/world/item/registry/ItemRegistry.h"
#include "mc/world/item/NetworkItemStackDescriptor.h"
#include "mc/world/inventory/network/ItemStackRequestData.h"

namespace CreativeContentPacketSerialize {

void Serialize_594(CreativeContentPacket* pkt, BinaryStream& stream) {
    if (!pkt->mWriteEntries.empty()) {
        stream.writeUnsignedVarInt(static_cast<uint>(pkt->mWriteEntries.size()));
        for (auto& key : pkt->mWriteEntries) {
            stream.writeUnsignedVarInt(key.getCreativeNetId().mRawId);
            NetworkItemInstanceDescriptor networkitem(key.getItemInstance());
            int16 id = networkitem.getId();
            int16 stackSize = networkitem.getStackSize();
            auto itemRegistry = ItemRegistryManager::getItemRegistry()._lockRegistry();
            auto item = itemRegistry->getItem(id);
            
            if (!item.expired() && networkitem.isValid(true)) {
                if (id == -646) {
                    stream.writeVarInt(160);
                    stream.writeUnsignedShort(stackSize);
                    stream.writeUnsignedVarInt(4);
                } else {
                    stream.writeVarInt(id);
                    stream.writeUnsignedShort(stackSize);
                    stream.writeUnsignedVarInt(!networkitem.getBlock() ? networkitem.getAuxValue() : 0);
                }
                stream.writeVarInt(GlobalBlockP->getBlockRuntimeIdFromMain(pkt->mProtocolVersion, networkitem.mBlockRuntimeId));
                stream.writeString(networkitem.mUserDataBuffer);
            } else {
                stream.writeVarInt(0);
            }
        }
    }
}

void Serialize_630(CreativeContentPacket* pkt, BinaryStream& stream) {
    if (!pkt->mWriteEntries.empty()) {
        stream.writeUnsignedVarInt(static_cast<uint>(pkt->mWriteEntries.size()));
        for (auto& key : pkt->mWriteEntries) {
            uint creativeId = key.getCreativeNetId().mRawId;
            stream.writeUnsignedVarInt(creativeId);
            NetworkItemInstanceDescriptor networkitem(key.getItemInstance());
            int16 id = networkitem.getId();
            int16 stackSize = networkitem.getStackSize();
            auto itemRegistry = ItemRegistryManager::getItemRegistry()._lockRegistry();
            auto item = itemRegistry->getItem(id);

            if (!item.expired() && networkitem.isValid(true)) {
                int16 finalId = id;
                switch (creativeId) {
                    case 2: finalId = -739; break;
                    case 3: finalId = -740; break;
                    case 4: finalId = -741; break;
                    case 5: finalId = -742; break;
                    case 6: finalId = -743; break;
                    case 442: finalId = -590; break;
                    case 443: finalId = -592; break;
                    case 444: finalId = -594; break;
                    case 447: finalId = -591; break;
                    case 448: finalId = -593; break;
                    case 449: finalId = -595; break;
                }
                stream.writeVarInt(finalId);
                stream.writeUnsignedShort(stackSize);
                stream.writeUnsignedVarInt(!networkitem.getBlock() ? networkitem.getAuxValue() : 0);
                stream.writeVarInt(GlobalBlockP->getBlockRuntimeIdFromMain(pkt->mProtocolVersion, networkitem.mBlockRuntimeId));
                stream.writeString(networkitem.mUserDataBuffer);
            } else {
                stream.writeVarInt(0);
            }
        }
    }
}

} 

LL_AUTO_TYPE_INSTANCE_HOOK(
    CreativeContentPacketWrite,
    ll::memory::HookPriority::Normal,
    CreativeContentPacket,
    "?write@CreativeContentPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& stream
) {
    switch (this->mProtocolVersion) {
        case 594: CreativeContentPacketSerialize::Serialize_594(this, stream); break;
        case 630: CreativeContentPacketSerialize::Serialize_630(this, stream); break;
        default: origin(stream); break;
    }
}

LL_AUTO_TYPE_STATIC_HOOK(
    _handleRequestData,
    ll::memory::HookPriority::Normal,
    ItemStackNetManagerServer,
    "?_handleRequestData@ItemStackNetManagerServer@@AEAAXAEAV?$vector@UItemStackResponseInfo@@V?$allocator@UItemStackResponseInfo@@@std@@@std@@PEBVItemStackRequestData@@@Z",
    void,
    ItemStackNetManagerServer* a1,
    std::vector<struct ItemStackResponseInfo>& a2,
    ItemStackRequestData const* a3
) {
    if (a3) {
        auto& actions = const_cast<std::vector<std::unique_ptr<class ItemStackRequestAction>>&>(a3->getActions());
        actions.erase(
            std::remove_if(actions.begin(), actions.end(), [](auto const& ptr) { return ptr == nullptr; }),
            actions.end()
        );
    }
    origin(a1, a2, a3);
}
