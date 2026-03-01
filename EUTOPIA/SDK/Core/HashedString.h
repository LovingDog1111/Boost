#pragma once
#include <string>
#include <memory>

class HashedString {
public:
	uint64_t hash;
	std::string str;
	mutable HashedString const* lastMatch;

    [[nodiscard]] constexpr static uint64_t computeHash(std::string_view str) {
        if (str.empty()) return 0;
        uint64_t hash = 0xCBF29CE484222325ULL;
        for (char s : str) {
            hash = s ^ (0x100000001B3ULL * hash);
        }
        return hash;
    }

    //HashedString(std::string_view strView) {
    //    hash = computeHash(strView);
    //    str = strView;
    //    lastMatch = nullptr;
    //}

    HashedString(uint64_t inputhash, std::string inputtext) {
        memset(this, 0x0, sizeof(HashedString));
        this->hash = inputhash;
        this->str = inputtext;
    }

    HashedString(const std::string& text) {
        memset(this, 0x0, sizeof(HashedString));
        this->str = text;
        this->computeHash();
    }

    HashedString() {
        memset(this, 0x0, sizeof(HashedString));
    }

    void setText(std::string text) {
        this->str = text;
        this->computeHash();
    }

    void computeHash() {
        hash = 0xCBF29CE484222325i64;
        if (this->str.length() <= 0)
            return;
        char* textP = this->str.data();
        auto c = *textP;

        do {
            hash = c ^ 0x100000001B3i64 * hash;
            c = *++textP;
        } while (*textP);
    }

    bool operator==(HashedString& rhs) {
        if (this->str == rhs.str) {
            return ((this->lastMatch == &rhs) && (rhs.lastMatch == this));
        }
        return false;
    }
    bool operator!=(HashedString& rhs) {
        return !(*this == rhs);
    }
    bool operator<(HashedString& rhs) {
        if (this->hash < rhs.hash) {
            return true;
        }
        if (this->hash <= rhs.hash) {
            return (strcmp(this->str.data(), rhs.str.data()) < 0);
        }
        return false;
    }

};