#pragma once
#include <sys/stat.h>
#include "Packets/Packet.h"


class MinecraftPacket {
public:
	static std::shared_ptr<Packet> createPacket(PacketID id) {
		static auto address= MemoryUtil::findSignature("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 48 8B D9 48 89 4C 24 ? 33 F6");
		using func_t = std::shared_ptr<Packet>(__cdecl*)(PacketID);
		static func_t Func = reinterpret_cast<func_t>(address);
		return Func(id);
	}
    template <typename T>
    static std::shared_ptr<T> createPacket() {
        // throw error if the type doesn't have an ID
        static_assert(std::is_base_of_v<Packet, T>,
                      "T must inherit from Packet, and have a static ID member");
        const PacketID id = T::ID;
        return std::reinterpret_pointer_cast<T>(createPacket(id));
    }
};