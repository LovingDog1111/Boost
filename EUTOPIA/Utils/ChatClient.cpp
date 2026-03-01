#include "ChatClient.h"
#include "Logger.h"

ChatClient::ChatClient(const std::string& host, int port) : host_(host), port_(port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        logF("WSAStartup failed: %d", WSAGetLastError());
    }
}

ChatClient::~ChatClient() {
    disconnect();
    WSACleanup();
}

bool ChatClient::connectToServer() {
    if (running_) return true;

    sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_ == INVALID_SOCKET) {
        logF("Socket creation failed: %d", WSAGetLastError());
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<u_short>(port_));
    if (InetPtonA(AF_INET, host_.c_str(), &serverAddr.sin_addr) != 1) {
        logF("Invalid server address: %s", host_.c_str());
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
        return false;
    }

    if (connect(sock_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        logF("Connect failed: %d", WSAGetLastError());
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
        return false;
    }

    running_ = true;
    recvThread_ = std::thread(&ChatClient::recvLoop, this);
    return true;
}

void ChatClient::disconnect() {
    bool wasRunning = running_.exchange(false);
    if (sock_ != INVALID_SOCKET) {
    shutdown(sock_, SD_BOTH);
    closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }
    if (recvThread_.joinable()) {
        if (std::this_thread::get_id() == recvThread_.get_id()) {
            // Cannot join self – detach instead to avoid terminate
            recvThread_.detach();
        } else {
            recvThread_.join();
        }
    }
}

bool ChatClient::sendMessage(const std::string& msg) {
    if (!running_) return false;
    std::string data = msg + "\n";
    int sent = send(sock_, data.c_str(), static_cast<int>(data.size()), 0);
    if (sent == SOCKET_ERROR) {
        logF("Send failed: %d", WSAGetLastError());
        return false;
    }
    return true;
}

void ChatClient::recvLoop() {
    std::string buffer;
    char temp[1024];
    while (running_) {
        int received = recv(sock_, temp, sizeof(temp), 0);
        if (received <= 0) {
            break;
        }
        buffer.append(temp, received);
        size_t pos;
        while ((pos = buffer.find('\n')) != std::string::npos) {
            std::string line = buffer.substr(0, pos);
            buffer.erase(0, pos + 1);
            if (onMessage_) {
                onMessage_(line);
            }
        }
    }
    running_ = false;
} 