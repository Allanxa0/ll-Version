#include "ll/api/memory/Hook.h"
#include "mc/network/packet/CameraPresetsPacket.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/nbt/CompoundTag.h"

LL_AUTO_TYPE_INSTANCE_HOOK(
    CameraPresetsPacketWrite,
    ll::memory::HookPriority::Normal,
    CameraPresetsPacket,
    "?write@CameraPresetsPacket@@UEBAXAEAVBinaryStream@@@Z",
    void,
    BinaryStream& bs
) {
    if (this->mProtocolVersion >= 618) {
        origin(bs);
    } else {
        auto nbt = CompoundTag::fromSnbt(R"({"properties":[{"name":"minecraft:has_nectar","type":2}],"type":"minecraft:bee")");
        if (nbt) {
            bs.writeCompoundTag(*nbt);
        }
    }
}
