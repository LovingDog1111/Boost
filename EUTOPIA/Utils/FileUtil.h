#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace FileUtil {

	std::string getRoamingStatePath();
	std::string getClientPath();

	inline bool doesFilePathExist(const std::string& path) {
		return std::filesystem::exists(path);
	}

	inline bool createPath(const std::string& path) {
		return std::filesystem::create_directory(path);
	}

	inline bool deletePath(const std::string& path) {
		return std::filesystem::remove(path);
	}

	// Write raw binary data to file (overwrite). Returns true on success.
	inline bool writeBinary(const std::string& path, const std::string& data) {
		std::ofstream ofs(path, std::ios::binary);
		if (!ofs.is_open()) return false;
		ofs.write(data.data(), static_cast<std::streamsize>(data.size()));
		return ofs.good();
	}
}