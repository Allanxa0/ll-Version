#include "ll/api/memory/Hook.h"
#include "mc/network/packet/LoginPacket.h"
#include "mc/deps/core/utility/ReadOnlyBinaryStream.h"
#include "mc/common/SharedConstants.h"
#include "mc/network/ConnectionRequest.h"

LL_AUTO_TYPE_INSTANCE_HOOK(
    LoginPacketRead,
    ll::memory::HookPriority::Normal,
    LoginPacket,
    "?_read@LoginPacket@@EEAA?AV?$Result@XVerror_code@std@@@Bedrock@@AEAVReadOnlyBinaryStream@@@Z",
    Bedrock::Result<void>,
    ReadOnlyBinaryStream& stream
) {
    this->mClientNetworkVersion = SharedConstants::NetworkProtocolVersion;
    stream.getSignedBigEndianInt();
    
    std::string connectionRequestStr;
    stream.getString(connectionRequestStr);
    
    auto cr = ConnectionRequest::fromString(connectionRequestStr);
    this->mConnectionRequest = std::make_unique<ConnectionRequest>(cr);
    
    return Bedrock::Result<void>::success();
}
