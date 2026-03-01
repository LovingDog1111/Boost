#include "PlusUtils.h"

#include <Windows.h>
#include <wincrypt.h>
#include <wininet.h>

#include <algorithm>
#include <regex>
#include <string>
#include <vector>

std::string PlusUtils::fetchMachineId() {
    HW_PROFILE_INFO hwInfo;
    if(!GetCurrentHwProfile(&hwInfo))
        return "error";

    HCRYPTPROV prov;
    HCRYPTHASH hash;
    BYTE buffer[16];
    DWORD bufferLen = 16;
    char hexDigits[] = "0123456789abcdef";

    if(!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        return "error";
    if(!CryptCreateHash(prov, CALG_MD5, 0, 0, &hash)) {
        CryptReleaseContext(prov, 0);
        return "error";
    }

    DWORD guidLen = lstrlenW(hwInfo.szHwProfileGuid) * sizeof(WCHAR);
    if(!CryptHashData(hash, (BYTE*)hwInfo.szHwProfileGuid, guidLen, 0)) {
        CryptDestroyHash(hash);
        CryptReleaseContext(prov, 0);
        return "error";
    }
    if(!CryptGetHashParam(hash, HP_HASHVAL, buffer, &bufferLen, 0)) {
        CryptDestroyHash(hash);
        CryptReleaseContext(prov, 0);
        return "error";
    }

    char result[33];
    for(DWORD i = 0; i < bufferLen; i++) {
        result[i * 2] = hexDigits[buffer[i] >> 4];
        result[i * 2 + 1] = hexDigits[buffer[i] & 0xf];
    }
    result[32] = 0;

    CryptDestroyHash(hash);
    CryptReleaseContext(prov, 0);
    return std::string(result);
}

std::string PlusUtils::fetchPlusList(const std::string& url) {
    HINTERNET inet = InternetOpenA("PlusFetcher", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if(!inet)
        return "";
    HINTERNET file = InternetOpenUrlA(inet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if(!file) {
        InternetCloseHandle(inet);
        return "";
    }

    char buf[1024];
    DWORD read;
    std::string out;
    while(InternetReadFile(file, buf, sizeof(buf), &read) && read > 0)
        out.append(buf, read);

    InternetCloseHandle(file);
    InternetCloseHandle(inet);
    return out;
}

bool PlusUtils::checkPlus(const std::string& url) {
    std::string id = fetchMachineId();
    std::string list = fetchPlusList(url);
    if(list.empty())
        return false;

    std::vector<std::string> ids;
    std::regex rx("\"([a-fA-F0-9]+)\"");
    std::smatch m;
    auto start = list.cbegin();
    while(std::regex_search(start, list.cend(), m, rx)) {
        ids.push_back(m[1]);
        start = m.suffix().first;
    }

    return std::find(ids.begin(), ids.end(), id) != ids.end();
}


bool PlusUtils::isPlusActive() {
    return checkPlus("http://43.226.0.155:5000/get_hwids2");
}