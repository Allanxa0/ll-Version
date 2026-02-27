#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Symbol.h"
#include "mc/network/packet/CameraPresetsPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/nbt/CompoundTag.h"
#include <unordered_map>

extern uint64 GlobalGuid;
extern std::unordered_map<uint64, int> PlayerGuidMap;

using namespace ll::memory_literals;

LL_AUTO_TYPE_INSTANCE_HOOK(
    CameraPresetsPacketWrite,
    ll::memory::HookPriority::Normal,
    CameraPresetsPacket,
    "?write@CameraPresetsPacket@@UEBAXAEAVBinaryStream@@@Z"_sym,
    void,
    BinaryStream& bs
) {
    int clientProtocol = 898; 
    if (GlobalGuid != 0 && PlayerGuidMap.count(GlobalGuid)) {
        clientProtocol = PlayerGuidMap[GlobalGuid];
    }

    if (clientProtocol >= 859) {
        origin(bs);
    } else {
        auto nbt = CompoundTag::fromSnbt(R"({"properties":[{"name":"minecraft:has_nectar","type":2}],"type":"minecraft:bee")");
        if (nbt) {
            std::string nbtData = nbt->toNetworkNbt();
            bs.write(nbtData.data(), nbtData.size());
        }
    }
}
