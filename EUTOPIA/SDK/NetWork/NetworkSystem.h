#pragma once
#pragma once
#include "../../Utils/MemoryUtil.h"
#include "RakNet/RaknetConnector.h"
class RemoteConnectorComposite {
public:
	CLASS_MEMBER(RaknetConnector*, rakNet, 0x70);
};

class NetworkSystem {
public:
	CLASS_MEMBER(RemoteConnectorComposite*, remoteConnector, 0x90);
};
