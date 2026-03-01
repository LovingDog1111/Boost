#pragma once
#include <string>

namespace NetworkUtil {
bool canAccessInternet();
bool downloadFile(std::string name, std::string path, std::string url);
// Perform a simple HTTP GET request and return the body in `out`.
// Return true on success.
bool httpGet(const std::string& url, std::string& out);

bool sendPostRequest(const std::string& url, const std::string& data, const std::string& headers);
bool getRedirectUrl(const std::string& url, std::string& redirectUrl);
std::string urlEncode(const std::string& str);
}  // namespace NetworkUtil