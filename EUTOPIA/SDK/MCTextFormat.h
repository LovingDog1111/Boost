#pragma once
#include <string.h>
#include <string>
#include "../Utils/Logger.h"
namespace MCTF {
	extern char BASE_CHARACTER[2];
	extern char BLACK[4];
	extern char DARK_BLUE[4];
	extern char DARK_GREEN[4];
	extern char DARK_AQUA[4];
	extern char DARK_RED[4];
	extern char DARK_PURPLE[4];
	extern char GOLD[4];
	extern char GRAY[4];
	extern char ORANGE[4];
	extern char DARK_GRAY[4];
	extern char BLUE[4];
	extern char GREEN[4];
	extern char AQUA[4];
	extern char RED[4];
	extern char LIGHT_PURPLE[4];
	extern char YELLOW[4];
	extern char WHITE[4];

	extern char OBFUSCATED[4];
	extern char BOLD[4];
	extern char STRIKETHROUGH[4];
	extern char UNDERLINE[4];
	extern char ITALIC[4];
	extern char RESET[4];

	void initColor(char* variable, char color);
	void initMCTF();
}

struct MCString {
    char* data;       
    size_t size;         
    size_t capacity;      
    char pad[8];        

    MCString() : data(nullptr), size(0), capacity(0) {
        memset(pad, 0, sizeof(pad));
    }

    MCString(const char* str) {
        if (!str || strlen(str) == 0) {
            data = nullptr;
            size = 0;
            capacity = 0;
        }
        else {
            size_t len = strlen(str);
            data = new char[len + 1];
            memcpy(data, str, len);
            data[len] = '\0';
            size = len;
            capacity = len;
            //logF("MCString created: data=%p, size=%zu, str='%s'", data, size, str);
        }
        memset(pad, 0, sizeof(pad));
    }

    MCString(const std::string& str) {
        if (str.empty()) {
            data = nullptr;
            size = 0;
            capacity = 0;
        }
        else {
            size_t len = str.length();
            data = new char[len + 1];
            memcpy(data, str.c_str(), len);
            data[len] = '\0';
            size = len;
            capacity = len;
        }
        memset(pad, 0, sizeof(pad));
    }


    template<typename... Args>
    MCString(const char* format, Args... args) {

        int len = snprintf(nullptr, 0, format, args...);
        if (len > 0) {
            data = new char[len + 1];
            snprintf(data, len + 1, format, args...);
            size = len;
            capacity = len;
        }
        else {
            data = nullptr;
            size = 0;
            capacity = 0;
        }
        memset(pad, 0, sizeof(pad));
    }

    ~MCString() {
        if (data) {
            delete[] data;
            data = nullptr;
        }
    }

    MCString(const MCString& other) {
        if (other.empty()) {
            data = nullptr;
            size = 0;
            capacity = 0;
        }
        else {
            size_t len = other.size;
            data = new char[len + 1];
            memcpy(data, other.data, len);
            data[len] = '\0';
            size = len;
            capacity = len;
        }
        memset(pad, 0, sizeof(pad));
    }


    MCString(MCString&& other) noexcept {
        data = other.data;
        size = other.size;
        capacity = other.capacity;
        memset(pad, 0, sizeof(pad));

        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
    }


    MCString& operator=(const MCString& other) {
        if (this != &other) {
            if (data) {
                delete[] data;
            }
            if (other.empty()) {
                data = nullptr;
                size = 0;
                capacity = 0;
            }
            else {
                size_t len = other.size;
                data = new char[len + 1];
                memcpy(data, other.data, len);
                data[len] = '\0';
                size = len;
                capacity = len;
            }
            memset(pad, 0, sizeof(pad));
        }
        return *this;
    }


    MCString& operator=(MCString&& other) noexcept {
        if (this != &other) {
            if (data) {
                delete[] data;
            }
            data = other.data;
            size = other.size;
            capacity = other.capacity;
            memset(pad, 0, sizeof(pad));

            other.data = nullptr;
            other.size = 0;
            other.capacity = 0;
        }
        return *this;
    }


    MCString operator+(const MCString& other) const {
        MCString result;
        if (empty() && other.empty()) {
            return result;
        }

        size_t totalLen = size + other.size;
        result.data = new char[totalLen + 1];

        if (!empty()) {
            memcpy(result.data, data, size);
        }
        if (!other.empty()) {
            memcpy(result.data + size, other.data, other.size);
        }
        result.data[totalLen] = '\0';
        result.size = totalLen;
        result.capacity = totalLen;
        memset(result.pad, 0, sizeof(result.pad));

        return result;
    }

    MCString operator+(const std::string& other) const {
        return *this + MCString(other);
    }

    MCString operator+(const char* other) const {
        return *this + MCString(other);
    }

    MCString& operator+=(const MCString& other) {
        if (!other.empty()) {
            size_t newSize = size + other.size;
            char* newData = new char[newSize + 1];

            if (!empty()) {
                memcpy(newData, data, size);
            }
            memcpy(newData + size, other.data, other.size);
            newData[newSize] = '\0';

            if (data) {
                delete[] data;
            }
            data = newData;
            size = newSize;
            capacity = newSize;
        }
        return *this;
    }

    MCString& operator+=(const std::string& other) {
        return *this += MCString(other);
    }

    MCString& operator+=(const char* other) {
        return *this += MCString(other);
    }

    bool operator==(const MCString& other) const {
        if (size != other.size) return false;
        if (empty() && other.empty()) return true;
        return memcmp(data, other.data, size) == 0;
    }

    bool operator==(const std::string& other) const {
        return *this == MCString(other);
    }

    bool operator==(const char* other) const {
        return *this == MCString(other);
    }

    bool operator!=(const MCString& other) const {
        return !(*this == other);
    }

    bool operator<(const MCString& other) const {
        if (empty() && other.empty()) return false;
        if (empty()) return true;
        if (other.empty()) return false;

        size_t minLen = (size < other.size) ? size : other.size;
        int cmp = memcmp(data, other.data, minLen);
        if (cmp != 0) return cmp < 0;
        return size < other.size;
    }

    char& operator[](size_t index) {
        if (index >= size) {

            resize(index + 1);
        }
        return data[index];
    }

    const char& operator[](size_t index) const {
        return (index < size) ? data[index] : '\0';
    }


    template<typename... Args>
    static MCString format(const char* format, Args... args) {
        return MCString(format, args...);
    }


    std::string toStdString() const {
        if (data && size > 0) {
            return std::string(data, size);
        }
        return "";
    }

    const char* c_str() const {
        return data ? data : "";
    }

    size_t length() const {
        return size;
    }

    bool empty() const {
        return size == 0 || !data;
    }

    void clear() {
        if (data) {
            delete[] data;
            data = nullptr;
        }
        size = 0;
        capacity = 0;
    }

    void resize(size_t newSize) {
        if (newSize == size) return;

        if (newSize == 0) {
            clear();
            return;
        }

        char* newData = new char[newSize + 1];
        if (data) {
            size_t copySize = (newSize < size) ? newSize : size;
            memcpy(newData, data, copySize);
            delete[] data;
        }


        if (newSize > size) {
            memset(newData + size, 0, newSize - size);
        }

        newData[newSize] = '\0';
        data = newData;
        size = newSize;
        capacity = newSize;
    }

    size_t find(const char* str, size_t pos = 0) const {
        if (!str || pos >= size) return std::string::npos;

        const char* found = strstr(data + pos, str);
        return found ? (found - data) : std::string::npos;
    }

    size_t find(const MCString& str, size_t pos = 0) const {
        return find(str.c_str(), pos);
    }

    size_t find(const std::string& str, size_t pos = 0) const {
        return find(str.c_str(), pos);
    }

    MCString substr(size_t pos = 0, size_t len = std::string::npos) const {
        if (pos >= size) return MCString();

        size_t actualLen = (len == std::string::npos || pos + len > size) ? size - pos : len;
        MCString result;
        result.data = new char[actualLen + 1];
        memcpy(result.data, data + pos, actualLen);
        result.data[actualLen] = '\0';
        result.size = actualLen;
        result.capacity = actualLen;
        memset(result.pad, 0, sizeof(result.pad));

        return result;
    }

    static MCString fromMCString(void* mcStringPtr) {
        MCString result;
        if (mcStringPtr) {
            result.data = *reinterpret_cast<char**>(mcStringPtr);
            result.size = *reinterpret_cast<size_t*>(reinterpret_cast<char*>(mcStringPtr) + 8);
            result.capacity = *reinterpret_cast<size_t*>(reinterpret_cast<char*>(mcStringPtr) + 16);
            memset(result.pad, 0, sizeof(result.pad));
        }
        return result;
    }
};

inline MCString operator+(const std::string& lhs, const MCString& rhs) {
    return MCString(lhs) + rhs;
}

inline MCString operator+(const char* lhs, const MCString& rhs) {
    return MCString(lhs) + rhs;
}

inline bool operator==(const std::string& lhs, const MCString& rhs) {
    return MCString(lhs) == rhs;
}

inline bool operator==(const char* lhs, const MCString& rhs) {
    return MCString(lhs) == rhs;
}


struct MCOptional {
    bool has_value;      
    char pad1[7];         
    MCString value;      

    MCOptional() : has_value(false) {
        memset(pad1, 0, sizeof(pad1));
    }

    MCOptional(const std::string& str) : has_value(true) {
        memset(pad1, 0, sizeof(pad1));
        value = MCString(str);
    }


    static MCOptional nullopt() {
        return MCOptional();
    }


    bool hasValue() const {
        return has_value;
    }

    std::string getValue() const {
        return has_value ? value.toStdString() : "";
    }
};