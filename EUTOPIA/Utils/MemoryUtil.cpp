#include "MemoryUtil.h"
#include "Logger.h"
#include <libhat/Scanner.hpp>
#include <libhat/process.hpp>
#include <vector>
#include <future>
#include <thread>
#include <sstream>
#include <iomanip>
#include <cstring>
uintptr_t MemoryUtil::getGameAddress() {
	static uintptr_t address = reinterpret_cast<uintptr_t>(GetModuleHandleA("Minecraft.Windows.exe"));
	return address;
}

uintptr_t MemoryUtil::findSignature(std::string_view sig) {
	auto address = hat::parse_signature(sig);
	if (!address.has_value()) {
		logF("Sig dead: %s", sig.data())
		return 0;
	}
	auto signature = address.value();

	const auto module = hat::process::get_process_module();
	auto result = hat::find_pattern(signature, ".text", module);

	if (result.has_result())
		return reinterpret_cast<uintptr_t>(result.get());

	logF("Sig dead: %s", sig.data());
	return 0;
}

uintptr_t** MemoryUtil::getVtableFromSig(std::string_view sig) {
	uintptr_t address = findSignature(sig);

	if (address == 0x0) 
		return nullptr;

	int finalOffset = *reinterpret_cast<int*>((address + 3));
	return reinterpret_cast<uintptr_t**>(address + finalOffset + 7);
}

int MemoryUtil::resolveOffset(std::string_view sig, int relativeOffset, bool isIndex) {
    uintptr_t address = findSignature(sig);
    if (address == 0) {
        logF("[resolveOffset] signature not found: %s", sig.data());
        return 0;
    }


    int value = *reinterpret_cast<int*>(address + relativeOffset);

    if (isIndex) {
        value /= 8;
    }

    return value;
}



void MemoryUtil::writeBytes(uintptr_t ptr, const std::vector<unsigned char>& bytes, size_t size) {
    DWORD oldProtect;
    VirtualProtect(reinterpret_cast<void*>(ptr), size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(reinterpret_cast<void*>(ptr), bytes.data(), size);
    VirtualProtect(reinterpret_cast<void*>(ptr), size, oldProtect, &oldProtect);
}

void MemoryUtil::writeBytes(uintptr_t ptr, const void* bytes, size_t size) {
    DWORD oldProtect;
    VirtualProtect(reinterpret_cast<void*>(ptr), size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(reinterpret_cast<void*>(ptr), bytes, size);
    VirtualProtect(reinterpret_cast<void*>(ptr), size, oldProtect, &oldProtect);
}

void MemoryUtil::writeBytes(uintptr_t ptr, const std::vector<unsigned char>& bytes) {
    writeBytes(ptr, bytes, bytes.size());
}

std::vector<unsigned char> MemoryUtil::readBytes(uintptr_t ptr, size_t size) {
    std::vector<unsigned char> buffer(size);
    DWORD oldProtect;
    VirtualProtect(reinterpret_cast<void*>(ptr), size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(buffer.data(), reinterpret_cast<void*>(ptr), size);
    VirtualProtect(reinterpret_cast<void*>(ptr), size, oldProtect, &oldProtect);
    return buffer;
}

void MemoryUtil::ReadBytes(void* address, void* buffer, size_t size) {
    DWORD oldProtect;
    VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(buffer, address, size);
    VirtualProtect(address, size, oldProtect, &oldProtect);
}

void MemoryUtil::setProtection(uintptr_t ptr, size_t size, DWORD protection) {
    DWORD oldProtect;
    VirtualProtect(reinterpret_cast<void*>(ptr), size, protection, &oldProtect);
}

void MemoryUtil::NopBytes(uintptr_t address, size_t size) {
    if (address == 0) return;
    DWORD oldProtect;
    VirtualProtect(reinterpret_cast<void*>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memset(reinterpret_cast<void*>(address), 0x90, size);
    VirtualProtect(reinterpret_cast<void*>(address), size, oldProtect, &oldProtect);
}

std::string MemoryUtil::bytesToHex(char* bytes, int length) {
    std::stringstream ss;
    for (int i = 0; i < length; ++i) {
        ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (0xFF & bytes[i]);
        if (i + 1 < length)
            ss << " ";
    }
    return ss.str();
}

#define INRANGE(x,a,b) ((x) >= (a) && (x) <= (b))
#define getBits(x)  (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? (x - '0') : 0))
#define getByte(x)  (getBits((x)[0]) << 4 | getBits((x)[1]))

std::vector<uintptr_t> MemoryUtil::findPattern(const std::string& pattern) {
    std::vector<uintptr_t> results;
    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &moduleInfo, sizeof(MODULEINFO)))
        return results;

    uintptr_t start = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
    uintptr_t end = start + moduleInfo.SizeOfImage;

    const char* pat = pattern.c_str();
    for (uintptr_t pCur = start; pCur < end; ++pCur) {
        if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat)) {
            if (!*pat) results.push_back(pCur);
            if (!pat[2]) results.push_back(pCur);

            if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
                pat += 3;
            else
                pat += 2;
        } else {
            pat = pattern.c_str();
        }
    }
    return results;
}

static uintptr_t findStringInRangeInternal(const std::string& target, uintptr_t start, uintptr_t end) {
    const size_t len = target.size();
    const char* tgt = target.c_str();
    for (uintptr_t addr = start; addr < end; ++addr) {
        if (memcmp(reinterpret_cast<const char*>(addr), tgt, len) == 0)
            return addr;
    }
    return 0;
}

uintptr_t MemoryUtil::findString(const std::string& str) {
    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &moduleInfo, sizeof(MODULEINFO)))
        return 0;
    uintptr_t start = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
    uintptr_t end = start + moduleInfo.SizeOfImage;

    const size_t threadCount = std::thread::hardware_concurrency();
    const uintptr_t range = (end - start) / threadCount;

    std::vector<std::future<uintptr_t>> futures;
    for (size_t i = 0; i < threadCount; ++i) {
        uintptr_t s = start + i * range;
        uintptr_t e = (i == threadCount - 1) ? end : s + range;
        futures.emplace_back(std::async(std::launch::async, findStringInRangeInternal, str, s, e));
    }
    for (auto& f : futures) {
        uintptr_t res = f.get();
        if (res != 0) return res;
    }
    return 0;
}

static uintptr_t findReferenceInRangeInternal(uintptr_t ptr, uintptr_t start, uintptr_t end) {
    for (uintptr_t addr = start; addr < end; ++addr) {
        uint8_t* bytePtr = reinterpret_cast<uint8_t*>(addr);
        // call rel32
        if (bytePtr[0] == 0xE8) {
            int32_t offset = *reinterpret_cast<int32_t*>(addr + 1);
            uintptr_t target = addr + offset + 5;
            if (target == ptr) return addr;
        }
        // lea r??,[rip+rel32]
        if (bytePtr[0] == 0x48 && bytePtr[1] == 0x8D) {
            int32_t offset = *reinterpret_cast<int32_t*>(addr + 3);
            uintptr_t target = addr + offset + 7;
            if (target == ptr) return addr;
        }
    }
    return 0;
}

uintptr_t MemoryUtil::findReference(uintptr_t address) {
    if (address == 0) {
        return 0;
    }
    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &moduleInfo, sizeof(MODULEINFO)))
        return 0;
    uintptr_t start = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
    uintptr_t end = start + moduleInfo.SizeOfImage;

    const size_t threadCount = std::thread::hardware_concurrency();
    const uintptr_t range = (end - start) / threadCount;

    std::vector<std::future<uintptr_t>> futures;
    for (size_t i = 0; i < threadCount; ++i) {
        uintptr_t s = start + i * range;
        uintptr_t e = (i == threadCount - 1) ? end : s + range;
        futures.emplace_back(std::async(std::launch::async, findReferenceInRangeInternal, address, s, e));
    }
    for (auto& f : futures) {
        uintptr_t res = f.get();
        if (res != 0) return res;
    }
    return 0;
}

std::vector<uintptr_t> MemoryUtil::findReferences(uintptr_t address) {
    std::vector<uintptr_t> results;
    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &moduleInfo, sizeof(MODULEINFO)))
        return results;
    uintptr_t start = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
    uintptr_t end = start + moduleInfo.SizeOfImage;

    for (uintptr_t addr = start; addr < end; ++addr) {
        uint8_t* bytePtr = reinterpret_cast<uint8_t*>(addr);
        if (bytePtr[0] == 0xE8) {
            int32_t offset = *reinterpret_cast<int32_t*>(addr + 1);
            uintptr_t target = addr + offset + 5;
            if (target == address) results.push_back(addr);
        }
        if (bytePtr[0] == 0x48 && bytePtr[1] == 0x8D) {
            int32_t offset = *reinterpret_cast<int32_t*>(addr + 3);
            uintptr_t target = addr + offset + 7;
            if (target == address) results.push_back(addr);
        }
    }
    return results;
}

uintptr_t MemoryUtil::getTopOfFunction(uintptr_t address) {
    if (address == 0) {
        return 0;
    }
    uintptr_t current = address;
    while (true) {
        uint8_t opcode = *reinterpret_cast<uint8_t*>(current);
        if (opcode == 0xCC || opcode == 0xC3) {
            if ((current + 1) % 16 == 0)
                return current + 1;
        }
        --current;
    }
}

int32_t MemoryUtil::GetRelativeAddress(uintptr_t address, uintptr_t target) {
    return static_cast<int32_t>(static_cast<uintptr_t>(target) - static_cast<uintptr_t>(address) - 4);
}

uintptr_t MemoryUtil::getAddressByIndex(uintptr_t _this, int index) {
    const auto vtable = *reinterpret_cast<uintptr_t**>(_this);
    return vtable[index];
}

uintptr_t MemoryUtil::GetVTableFunction(void* _this, int index) {
    uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(_this);
    return vtable[index];
}

