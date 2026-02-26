#include "ll/api/memory/Hook.h"
#include "ll/core/LeviLamina.h"
#include "mc/network/NetworkIdentifier.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/packet/RequestNetworkSettingsPacket.h"
#include "mc/network/LoopbackPacketSender.h"
#include "mc/network/NetworkSystem.h"
#include "mc/network/Packet.h"
#include "mc/common/SharedConstants.h"
#include "mc/network/NetworkConnection.h"
#include <unordered_map>
#include <unordered_set>

std::unordered_map<uint64, int> PlayerGuidMap;
std::unordered_set<int>         SupportProtocolList = {594, 618, 622, 630};

LL_AUTO_TYPE_INSTANCE_HOOK(
    RequestNetworkSettingsPacketHandle,
    ll::memory::HookPriority::Normal,
    ServerNetworkHandler,
    "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVRequestNetworkSettingsPacket@@@Z",
    void,
    NetworkIdentifier const& a1,
    RequestNetworkSettingsPacket const& a2
) {
    if (SupportProtocolList.count(a2.mClientNetworkVersion)) {
        ll::getLogger().info("Client Connecting, Guid: {}, Protocol Version: {}", a1.mGuid.g, a2.mClientNetworkVersion);
        PlayerGuidMap[a1.mGuid.g] = a2.mClientNetworkVersion;
        const_cast<RequestNetworkSettingsPacket&>(a2).mClientNetworkVersion = SharedConstants::NetworkProtocolVersion;
    } else {
        ll::getLogger().info("Client Disconnected, Guid: {}, Unsupported Protocol Version: {}", a1.mGuid.g, a2.mClientNetworkVersion);
    }
    origin(a1, a2);
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    UpdateBlockForAllPlayers,
    ll::memory::HookPriority::Normal,
    LoopbackPacketSender,
    "?sendToClients@LoopbackPacketSender@@UEAAXAEBV?$vector@UNetworkIdentifierWithSubId@@V?$allocator@UNetworkIdentifierWithSubId@@@std@@@std@@AEBVPacket@@@Z",
    void,
    std::vector<NetworkIdentifierWithSubId> const& a1,
    Packet const& a2
) {
    for (auto const& pl : a1) { 
        this->sendToClient(pl.id, a2, pl.subClientId); 
    }
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    ServerNetworkHandlerAllowIncomingPacketId,
    ll::memory::HookPriority::Normal,
    ServerNetworkHandler,
    "?allowIncomingPacketId@ServerNetworkHandler@@UEAA_NAEBVNetworkIdentifier@@W4MinecraftPacketIds@@@Z",
    bool,
    NetworkIdentifier const& a1,
    ::MinecraftPacketIds packetId
) {
    if ((int)packetId >= 308) {
        std::string message = "disconnectionScreen.unknownPacket";
        this->disconnectClient(a1, SubClientId::PrimaryClient, Connection::DisconnectFailReason(48), message, false);
        return false;
    }
    return origin(a1, packetId);
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    PacketSend,
    ll::memory::HookPriority::Normal,
    NetworkSystem,
    "?send@NetworkSystem@@QEAAXAEBVNetworkIdentifier@@AEBVPacket@@W4SubClientId@@@Z",
    void,
    NetworkIdentifier const& a1,
    Packet const& a2,
    SubClientId a3
) {
    const_cast<Packet&>(a2).mProtocolVersion = PlayerGuidMap[a1.mGuid.g];
    origin(a1, a2, a3);
}

uint64 GlobalGuid;

LL_AUTO_TYPE_INSTANCE_HOOK(
    NetworkSystemSortAndPacketizeEvents,
    ll::memory::HookPriority::Normal,
    NetworkSystem,
    "?_sortAndPacketizeEvents@NetworkSystem@@AEAA_NAEAVNetworkConnection@@V?$time_point@Usteady_clock@chrono@std@@V?$duration@_JU?$ratio@$00$0DLJKMKAA@@std@@@23@@chrono@std@@@Z",
    bool,
    NetworkConnection& a1,
    std::chrono::steady_clock::time_point a2
) {
    GlobalGuid = 0;
    auto* id = reinterpret_cast<NetworkIdentifier*>(&a1);
    bool result;
    if (id) {
        GlobalGuid = id->mGuid.g;
        result = origin(a1, a2);
        GlobalGuid = 0;
    } else {
        result = origin(a1, a2);
    }
    return result;
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    PacketRead,
    ll::memory::HookPriority::Normal,
    Packet,
    "?read@Packet@@UEAA?AV?$Result@XVerror_code@std@@@Bedrock@@AEAVReadOnlyBinaryStream@@@Z",
    Bedrock::Result<void>,
    ReadOnlyBinaryStream& a1
) {
    if (GlobalGuid != 0) {
        if (PlayerGuidMap.count(GlobalGuid)) { 
            this->mProtocolVersion = PlayerGuidMap[GlobalGuid]; 
        }
    }
    return origin(a1);
}
