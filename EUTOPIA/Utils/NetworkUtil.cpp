#include "NetworkUtil.h"

#include <Windows.h>
#include <winhttp.h>
#include <wininet.h>

#include <cstdio>

#include "FileUtil.h"
#pragma comment(lib, "winhttp.lib")

bool NetworkUtil::canAccessInternet() {
    char url[] = "https://www.google.com/";
    return InternetCheckConnectionA(url, FLAG_ICC_FORCE_CONNECTION, 0);
}

bool NetworkUtil::downloadFile(std::string name, std::string path, std::string url) {
    std::string filePath = path + name;

    HINTERNET webH = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
    HINTERNET urlFile;

    if(!webH)
        return false;

    urlFile = InternetOpenUrlA(webH, url.c_str(), NULL, NULL, NULL, NULL);

    if(!urlFile) {
        InternetCloseHandle(webH);
        return false;
    }

    std::ofstream outputFile(filePath, std::ios::binary);

    if(!outputFile.is_open()) {
        InternetCloseHandle(webH);
        InternetCloseHandle(urlFile);
        return false;
    }

    char buffer[2000];
    DWORD bytesRead;

    do {
        InternetReadFile(urlFile, buffer, 2000, &bytesRead);
        outputFile.write(buffer, bytesRead);
        memset(buffer, 0, 2000);
    } while(bytesRead);

    outputFile.close();
    InternetCloseHandle(webH);
    InternetCloseHandle(urlFile);

    return true;
}

// Helper: WinHTTP GET (UTF-8 URL) fallback
static bool httpGetWinHttp(const std::string& url, std::string& out) {
    out.clear();
    // Convert to wide string
    int wlen = MultiByteToWideChar(CP_UTF8, 0, url.c_str(), -1, NULL, 0);
    if(wlen <= 0)
        return false;
    std::wstring wurl(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, url.c_str(), -1, &wurl[0], wlen);

    URL_COMPONENTS comps{0};
    comps.dwStructSize = sizeof(comps);
    comps.dwSchemeLength = comps.dwHostNameLength = comps.dwUrlPathLength =
        comps.dwExtraInfoLength = (DWORD)-1;
    if(!WinHttpCrackUrl(wurl.c_str(), 0, 0, &comps))
        return false;

    std::wstring host(comps.lpszHostName, comps.dwHostNameLength);
    std::wstring path(comps.lpszUrlPath, comps.dwUrlPathLength);
    if(comps.lpszExtraInfo)
        path += std::wstring(comps.lpszExtraInfo, comps.dwExtraInfoLength);

    HINTERNET hSession =
        WinHttpOpen(L"Mozilla/5.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    if(!hSession)
        return false;

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), comps.nPort, 0);
    if(!hConnect) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    DWORD flags = WINHTTP_FLAG_BYPASS_PROXY_CACHE;
    if(comps.nScheme == INTERNET_SCHEME_HTTPS)
        flags |= WINHTTP_FLAG_SECURE;

    HINTERNET hRequest =
        WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER,
                           WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if(!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Ignore certificate errors
    DWORD secFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
                     SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
    WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &secFlags, sizeof(secFlags));

    bool ok = false;
    if(WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0) &&
       WinHttpReceiveResponse(hRequest, NULL)) {
        DWORD dwSize = 0;
        do {
            if(!WinHttpQueryDataAvailable(hRequest, &dwSize))
                break;
            if(dwSize == 0) {
                ok = true;
                break;
            }
            std::vector<char> buffer(dwSize);
            DWORD dwDownloaded = 0;
            if(!WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded))
                break;
            out.append(buffer.data(), dwDownloaded);
        } while(dwSize > 0);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return ok;
}

bool NetworkUtil::httpGet(const std::string& url, std::string& out) {
    out.clear();

    // 尝试 WinInet 方法
    HINTERNET hSession =
        InternetOpenA("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36",
                      INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(!hSession) {
        // 如果 WinInet 失败，使用 WinHTTP 备用方案
        return httpGetWinHttp(url, out);
    }

    DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
    if(url.find("https://") == 0) {
        flags |= INTERNET_FLAG_SECURE;
    }

    HINTERNET hUrl = InternetOpenUrlA(hSession, url.c_str(), NULL, 0, flags, 0);
    if(!hUrl) {
        InternetCloseHandle(hSession);
        return httpGetWinHttp(url, out);
    }

    // 检查 HTTP 状态码
    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    if(HttpQueryInfoA(hUrl, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode,
                      &statusSize, NULL)) {
        if(statusCode != 200) {
            // 如果不是成功状态码，记录并尝试备用方案
            InternetCloseHandle(hUrl);
            InternetCloseHandle(hSession);
            return httpGetWinHttp(url, out);
        }
    }

    char buffer[4096];
    DWORD bytesRead;
    bool success = false;

    while(InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        out.append(buffer, bytesRead);
        success = true;
    }

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hSession);

    // 如果下载的数据太小，可能是错误页面
    if(out.size() < 1000) {
        std::string backup_out;
        if(httpGetWinHttp(url, backup_out) && backup_out.size() > out.size()) {
            out = backup_out;
            return true;
        }
    }

    return success && !out.empty();
}

bool NetworkUtil::getRedirectUrl(const std::string& url, std::string& redirectUrl) {
    redirectUrl.clear();
    HINTERNET hSession = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(!hSession)
        return false;

    DWORD flags = INTERNET_FLAG_NO_AUTO_REDIRECT | INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE;
    HINTERNET hUrl = InternetOpenUrlA(hSession, url.c_str(), NULL, 0, flags, 0);
    if(!hUrl) {
        InternetCloseHandle(hSession);
        return false;
    }

    DWORD status = 0;
    DWORD len = sizeof(status);
    if(HttpQueryInfoA(hUrl, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &len, NULL)) {
        if(status >= 300 && status < 400) {
            char buffer[512];
            len = sizeof(buffer);
            if(HttpQueryInfoA(hUrl, HTTP_QUERY_LOCATION, buffer, &len, NULL)) {
                redirectUrl.assign(buffer, len);
            }
        }
    }

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hSession);
    return !redirectUrl.empty();
}

static inline bool isUnreserved(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' ||
           c == '_' || c == '.' || c == '~';
}

std::string NetworkUtil::urlEncode(const std::string& str) {
    std::string encoded;
    char buf[4];
    for(unsigned char c : str) {
        if(isUnreserved(c)) {
            encoded.push_back(c);
        } else {
            sprintf_s(buf, "%%%02X", c);
            encoded.append(buf);
        }
    }
    return encoded;
}
bool NetworkUtil::sendPostRequest(const std::string& url, const std::string& data,
                                  const std::string& headers) {
    HINTERNET hSession = nullptr;
    HINTERNET hConnect = nullptr;
    HINTERNET hRequest = nullptr;
    bool result = false;
    try {
        URL_COMPONENTS urlComp;
        ZeroMemory(&urlComp, sizeof(urlComp));
        urlComp.dwStructSize = sizeof(urlComp);
        urlComp.dwSchemeLength = (DWORD)-1;
        urlComp.dwHostNameLength = (DWORD)-1;
        urlComp.dwUrlPathLength = (DWORD)-1;
        urlComp.dwExtraInfoLength = (DWORD)-1;
        if(!WinHttpCrackUrl(std::wstring(url.begin(), url.end()).c_str(), (DWORD)url.length(), 0,
                            &urlComp)) {
            throw std::runtime_error("Failed to crack URL");
        }
        std::wstring hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);
        std::wstring urlPath(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
        hSession = WinHttpOpen(L"HWID Verifier/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                               WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if(!hSession)
            throw std::runtime_error("Failed to open session");
        hConnect = WinHttpConnect(hSession, hostName.c_str(), urlComp.nPort, 0);
        if(!hConnect)
            throw std::runtime_error("Failed to connect");
        hRequest = WinHttpOpenRequest(hConnect, L"POST", urlPath.c_str(), NULL, WINHTTP_NO_REFERER,
                                      WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
        if(!hRequest)
            throw std::runtime_error("Failed to open request");
        if(!headers.empty()) {
            if(!WinHttpAddRequestHeaders(hRequest,
                                         std::wstring(headers.begin(), headers.end()).c_str(),
                                         (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD)) {
                throw std::runtime_error("Failed to add headers");
            }
        }
        if(!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)data.c_str(),
                               (DWORD)data.size(), (DWORD)data.size(), 0)) {
            throw std::runtime_error("Failed to send request");
        }
        if(!WinHttpReceiveResponse(hRequest, NULL)) {
            throw std::runtime_error("Failed to receive response");
        }
        DWORD statusCode = 0;
        DWORD size = sizeof(statusCode);
        if(!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size,
                                WINHTTP_NO_HEADER_INDEX)) {
            throw std::runtime_error("Failed to query status code");
        }
        result = (statusCode == 200);
    } catch(const std::exception& e) {
    }
    if(hRequest)
        WinHttpCloseHandle(hRequest);
    if(hConnect)
        WinHttpCloseHandle(hConnect);
    if(hSession)
        WinHttpCloseHandle(hSession);

    return result;
}