#include "ll/api/memory/Hook.h"
#include "mc/network/LoopbackPacketSender.h"
#include "mc/network/Packet.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/entity/components/UserEntityIdentifierComponent.h"
#include <unordered_map>

extern std::unordered_map<uint64, int> PlayerGuidMap;

LL_AUTO_TYPE_INSTANCE_HOOK(
    CameraInstructionPacketSend,
    ll::memory::HookPriority::Normal,
    LoopbackPacketSender,
    "?sendToClient@LoopbackPacketSender@@UEAAXPEBVUserEntityIdentifierComponent@@AEBVPacket@@@Z",
    void,
    UserEntityIdentifierComponent const* a2,
    Packet const& a3
) {
    if (a2) {
        auto protocol = PlayerGuidMap[a2->mNetworkId.mGuid.g];
        if (protocol < 618) {
            if (a3.getId() == MinecraftPacketIds::CameraInstruction) {
                auto pkt = TextPacket::createTranslated(
                    "%packdiscoveryerror.invalid_capability_value",
                    {"%createWorldScreen.experimentalCameras"}
                );
                origin(a2, pkt);
                return;
            }
        }
    }
    origin(a2, a3);
}
