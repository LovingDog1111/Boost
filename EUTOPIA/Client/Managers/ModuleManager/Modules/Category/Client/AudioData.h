#pragma once
#include <cstdint>
#include <string>

namespace AudioData {
// Original three sounds

// New sounds
extern const uint8_t cash_hex[];
extern const uint8_t celestial_hex[];
extern const uint32_t celestial_hex_size;
extern const uint8_t celestial_off_hex[];
extern const uint32_t celestial_off_hex_size;

extern const uint32_t cash_hex_size;

// Helper function to get sound data by name
const uint8_t* getSoundData(const std::string& name, uint32_t& size);
}  // namespace AudioData