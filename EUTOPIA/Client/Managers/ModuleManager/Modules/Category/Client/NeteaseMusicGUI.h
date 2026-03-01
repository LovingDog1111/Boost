#pragma once
#include "../../ModuleBase/Module.h"
#include <windows.h>   // 定义 HINTERNET
#include <wininet.h>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>

class NeteaseMusicGUI : public Module {
public:
    NeteaseMusicGUI();
    void onDisable() override;
    void onD2DRender() override;
    void onKeyUpdate(int key, bool isDown) override;
    bool onMouseUpdate(Vec2<float> mousePos, char mouseButton, char isDown);
    void Render();
    // UI state management
    float size = 1.0f;
    bool dragging = false;
    Vec2<float> dragOffset = { 0.f, 0.f };
    Vec2<float> panelPos = { 150.f, 80.f };
    Vec2<float> panelSize = { 600.f, 400.f }; // smaller, more compact

    void savePanelState(nlohmann::json& j) const;
    void loadPanelState(const nlohmann::json& j);

    // Public interface for ClickGUI integration
    bool isSearchFocused() const { return searchFocused; }

private:



    // Data structures
    struct Song {
        int id;
        std::string name;
        std::string artist;
        std::string albumCover;
        bool vip;

        Song() : id(0), vip(false) {}
        Song(int i, const std::string& n, const std::string& a, const std::string& cover = "", bool v = false)
            : id(i), name(n), artist(a), albumCover(cover), vip(v) {
        }
    };

    // Core data
    std::vector<Song> songs;
    std::string searchQuery;
    int selectedSong = 0;
    int maxDisplaySongs = 12; // limit display count for performance

    // UI state
    bool searchFocused = false;
    bool isSearching = false;
    mutable bool searchBoxClicked = false; // For click feedback
    mutable std::chrono::steady_clock::time_point searchClickTime;

    // 音量控制
    float volume = 1.0f;                // 0.0 - 1.0
    bool draggingVolume = false;

    // Audio playback
    std::atomic_bool isPlaying{ false };
    std::atomic_bool isPaused{ false };
    std::thread audioThread;
    std::thread searchThread;
    std::atomic_bool terminating{ false };

    // ---------- 歌词 -----------
    struct LyricLine {
        double timeSec; // 时间戳（秒）
        std::string text;
    };
    std::vector<LyricLine> lyrics;
    int lyricIndex = -1; // 当前歌词下标
    int lastDisplayedIndex = -1; // 已在聊天栏显示的最后一句
    std::chrono::steady_clock::time_point playbackStart;
    double pausedAccum = 0.0; // 总暂停时间
    std::chrono::steady_clock::time_point pauseStart;

    void fetchLyrics(int songId);
    void renderLyricsOverlay();
    void updateLyricIndex();

    // Core functionality
    void renderUI();
    void handleSearch();
    void handlePlayback();
    void cleanup();

    // Helper functions
    bool shouldRender() const;
    void startSearch(const std::string& keyword);
    void playSong(int songIndex);
    void stopPlayback();
    void togglePlayPause();
    void nextSong();
    void previousSong();

    // Network
    void searchSongs(const std::string& keyword);

    // Thread management
    void joinThreads();
};