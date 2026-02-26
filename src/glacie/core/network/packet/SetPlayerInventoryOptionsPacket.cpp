#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/network/Packet.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/deps/core/utility/ReadOnlyBinaryStream.h"
#include "mc/network/MinecraftPacketIds.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/server/ServerPlayer.h"
#include "mc/network/NetworkIdentifier.h"
#include <unordered_map>

struct InventorySettings {
    int  leftTab;
    int  rightTab;
    bool filtering;
    int  layout;
    int  craftingLayout;
};

std::unordered_map<long long, InventorySettings> PlayerInventorySettings;

class SetPlayerInventoryOptionsPacket : public Packet {
public:
    int  leftTab;
    int  rightTab;
    bool filtering;
    int  layout;
    int  craftingLayout;

    SetPlayerInventoryOptionsPacket() : Packet() {}
    SetPlayerInventoryOptionsPacket(int lt, int rt, bool f, int l, int cl)
        : leftTab(lt), rightTab(rt), filtering(f), layout(l), craftingLayout(cl) {}

    virtual MinecraftPacketIds getId() const override { 
        return (MinecraftPacketIds)307; 
    }

    virtual std::string getName() const override { 
        return "SetPlayerInventoryOptionsPacket"; 
    }

    virtual void write(BinaryStream& bs) const override {
        bs.writeVarInt(leftTab);
        bs.writeVarInt(rightTab);
        bs.writeBool(filtering);
        bs.writeVarInt(layout);
        bs.writeVarInt(craftingLayout);
    }

    virtual Bedrock::Result<void> _read(ReadOnlyBinaryStream& rs) override {
        auto res1 = rs.getVarInt();
        auto res2 = rs.getVarInt();
        auto res3 = rs.getBool();
        auto res4 = rs.getVarInt();
        auto res5 = rs.getVarInt();

        if (res1 && res2 && res3 && res4 && res5) {
            leftTab = res1.value();
            rightTab = res2.value();
            filtering = res3.value();
            layout = res4.value();
            craftingLayout = res5.value();
            return Bedrock::Result<void>::success();
        }
        return Bedrock::Result<void>::error("Failed to read SetPlayerInventoryOptionsPacket");
    }
};

LL_AUTO_TYPE_STATIC_HOOK(
    CreatePackets,
    ll::memory::HookPriority::Normal,
    MinecraftPackets,
    "?createPacket@MinecraftPackets@@SA?AV?$shared_ptr@VPacket@@@std@@W4MinecraftPacketIds@@@Z",
    std::shared_ptr<Packet>,
    MinecraftPacketIds id
) {
    if ((int)id == 307) {
        return std::make_shared<SetPlayerInventoryOptionsPacket>();
    }
    return origin(id);
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    InventoryOptionsPacketHandle,
    ll::memory::HookPriority::Normal,
    ServerNetworkHandler,
    "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVPacket@@@Z",
    void,
    NetworkIdentifier const& id,
    Packet const& pkt
) {
    if ((int)pkt.getId() == 307) {
        auto const& iopkt = static_cast<SetPlayerInventoryOptionsPacket const&>(pkt);
        auto* player = this->getServerPlayer(id, SubClientId::PrimaryClient);
        if (player) {
            InventorySettings settings{
                iopkt.leftTab,
                iopkt.rightTab,
                iopkt.filtering,
                iopkt.layout,
                iopkt.craftingLayout
            };
            PlayerInventorySettings[player->getOrCreateUniqueID().id] = settings;
        }
    }
    origin(id, pkt);
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    SendJoinPacket,
    ll::memory::HookPriority::Normal,
    ServerPlayer,
    "?setLocalPlayerAsInitialized@ServerPlayer@@QEAAXXZ",
    void
) {
    origin();
    
    long long uid = this->getOrCreateUniqueID().id;
    if (PlayerInventorySettings.count(uid)) {
        auto const& s = PlayerInventorySettings[uid];
        SetPlayerInventoryOptionsPacket pkt(s.leftTab, s.rightTab, s.filtering, s.layout, s.craftingLayout);
        this->sendNetworkPacket(pkt);
    } else {
        SetPlayerInventoryOptionsPacket pkt(6, 2, true, 2, 2);
        this->sendNetworkPacket(pkt);
    }
}
