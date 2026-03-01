#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class ChatClient {
public:
    // Callback invoked when a line-based JSON message is received
    using MessageCallback = std::function<void(const std::string&)>;
private:
    std::string host_;
    int port_;
    SOCKET sock_ = INVALID_SOCKET;
    std::thread recvThread_;
    std::atomic_bool running_{ false };
    MessageCallback onMessage_;

    void recvLoop();
public:
    ChatClient(const std::string& host, int port);
    ~ChatClient();

    // Connect to server (non-blocking recv thread starts on success)
    bool connectToServer();
    // Graceful disconnect
    void disconnect();
    // Send a pre-formatted JSON string terminated by \n
    bool sendMessage(const std::string& msg);

    void setMessageCallback(MessageCallback cb) { onMessage_ = std::move(cb); }

    bool isRunning() const { return running_.load(); }
}; 