#pragma once
#include <string>

namespace Obf {
    inline std::string decode(const unsigned char* data, size_t len, unsigned char key = 0xAA) {
        std::string out;
        out.resize(len);
        for (size_t i = 0; i < len; ++i) {
            out[i] = static_cast<char>(data[i] ^ key);
        }
        return out;
    }

 // Pre-encoded common group names (key 0xAA)
 inline const std::string& STR_DEFAULT() {
     static const unsigned char enc[]{0xC2,0xCF,0xCC,0xCB,0xDF,0xC6,0xDE};
     static const std::string s = decode(enc, sizeof(enc));
     return s;
 }
 inline const std::string& STR_PREMIUM() {
     static const unsigned char enc[]{0xDA,0xD8,0xCF,0xC7,0xC3,0xDF,0xC7};
     static const std::string s = decode(enc, sizeof(enc));
     return s;
 }
 inline const std::string& STR_DEV() {
     static const unsigned char enc[]{0xEE,0xEF,0xFC};
     static const std::string s = decode(enc, sizeof(enc));
     return s;
 }
} 