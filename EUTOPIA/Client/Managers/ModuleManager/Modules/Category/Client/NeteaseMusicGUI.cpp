#include "NeteaseMusicGUI.h"

#include <Windows.h>

#include <algorithm>
#include <filesystem>

#include "../../../../../../Libs/json.hpp"
#include "../../../../../../Utils/NetworkUtil.h"
#include "../../../ModuleManager.h"
#include "ClickGUI.h"

// Audio library
#define MINIAUDIO_IMPLEMENTATION
#include <wininet.h>

#include <chrono>
#include <iomanip>
#include <sstream>

#include "../../../../../../Libs/miniaudio/miniaudio.h"

// Audio globals for simplified management
static ma_decoder* gDecoder = nullptr;
static ma_device* gDevice = nullptr;
// 音量 (0.0~1.0)
static std::atomic<float> gMasterVolume{1.0f};

static void ma_data_callback(ma_device* pDevice, void* pOutput, const void* pInput,
                             ma_uint32 frameCount) {
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if(pDecoder) {
        ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

        // 音量缩放
        float vol = gMasterVolume.load();
        if(vol < 0.f)
            vol = 0.f;
        if(vol > 1.f)
            vol = 1.f;

        ma_format fmt = pDevice->playback.format;
        ma_uint32 channels = pDevice->playback.channels;
        ma_uint32 sampleCount = frameCount * channels;

        if(vol < 0.999f) {
            if(fmt == ma_format_f32) {
                float* buf = (float*)pOutput;
                for(ma_uint32 i = 0; i < sampleCount; ++i) {
                    buf[i] *= vol;
                }
            } else if(fmt == ma_format_s16) {
                int16_t* buf = (int16_t*)pOutput;
                for(ma_uint32 i = 0; i < sampleCount; ++i) {
                    buf[i] = static_cast<int16_t>(buf[i] * vol);
                }
            } else if(fmt == ma_format_s32) {
                int32_t* buf = (int32_t*)pOutput;
                for(ma_uint32 i = 0; i < sampleCount; ++i) {
                    buf[i] = static_cast<int32_t>(buf[i] * vol);
                }
            }
        }
    }
    (void)pInput;  // 未使用
}

using json = nlohmann::json;

// Simplified API configuration
static const std::string API_BASE = "https://163api.qijieya.cn";
static const std::string PLAY_API = "https://node.api.xfabe.com/api/wangyi/music";
static const std::string PLAY_SUFFIX = "&quality=2";

NeteaseMusicGUI::NeteaseMusicGUI()
    : Module("Music", "Play music", Category::CLIENT) {
    songs.clear();
    selectedSong = 0;
    searchQuery = "";
    searchFocused = false;
    isSearching = false;
    isPlaying = false;
    isPaused = false;

    registerSetting(new SliderSetting<float>("Size", "UI Scale", &size, 1.0f, 0.7f, 1.5f));
}

void NeteaseMusicGUI::playSong(int songIndex) {
    if(songIndex < 0 || songIndex >= songs.size())
        return;

    stopPlayback();
    joinThreads();

    std::string songName = songs[songIndex].name + " - " + songs[songIndex].artist;
    GI::DisplayClientMessage("%s[Music]%s Fetching.. %s", MCTF::GOLD, MCTF::WHITE, songName.c_str());

    audioThread = std::thread([this, songIndex, songName]() {
        std::string id = std::to_string(songs[songIndex].id);
        std::string playUrl;
        std::string resp;

        std::string url = PLAY_API + "?id=" + id + PLAY_SUFFIX;
        if(NetworkUtil::httpGet(url, resp)) {
            try {
                auto pj = json::parse(resp);
                if(pj.contains("url") && pj["url"].is_string()) {
                    playUrl = pj["url"].get<std::string>();
                }
            } catch(...) {
            }
        }

        if(playUrl.empty()) {
            std::string playUrlRedirect;
            if(NetworkUtil::getRedirectUrl(PLAY_API + "?id=" + id + "&play=true",
                                           playUrlRedirect)) {
                playUrl = playUrlRedirect;
            }
        }

        GI::DisplayClientMessage("%s[Music]%s Downloading...", MCTF::GOLD, MCTF::WHITE);

        std::filesystem::path tmpPath = std::filesystem::temp_directory_path() / (id + ".mp3");

        HINTERNET hSession =
            InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if(!hSession) {
            GI::DisplayClientMessage("%s[Music]%s InternetOpenA failed", MCTF::RED, MCTF::WHITE);
            return;
        }
        HINTERNET hUrl = InternetOpenUrlA(hSession, playUrl.c_str(), NULL, 0,
                                          INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
        if(!hUrl) {
            GI::DisplayClientMessage("%s[Music]%s OpenUrl failed", MCTF::RED, MCTF::WHITE);
            InternetCloseHandle(hSession);
            return;
        }

        HANDLE hFile = CreateFileA(tmpPath.string().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
        if(hFile == INVALID_HANDLE_VALUE) {
            GI::DisplayClientMessage("%s[Music]%s File create failed", MCTF::RED, MCTF::WHITE);
            InternetCloseHandle(hUrl);
            InternetCloseHandle(hSession);
            return;
        }

        const DWORD BUF_SIZE = 64 * 1024;  // 64KB
        std::unique_ptr<char[]> buf(new char[BUF_SIZE]);
        DWORD bytesRead = 0, bytesWritten = 0;
        DWORD total = 0;
        bool started = false;
        const DWORD START_THRESHOLD = 512 * 1024;  // 512KB

        auto startPlayback = [&](const std::filesystem::path& path) -> bool {
            if(gDevice) {
                ma_device_stop(gDevice);
                ma_device_uninit(gDevice);
                delete gDevice;
                gDevice = nullptr;
            }
            if(gDecoder) {
                ma_decoder_uninit(gDecoder);
                delete gDecoder;
                gDecoder = nullptr;
            }
            gDecoder = new ma_decoder;
            if(ma_decoder_init_file(path.string().c_str(), NULL, gDecoder) != MA_SUCCESS) {
                delete gDecoder;
                gDecoder = nullptr;
                return false;
            }
            ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
            deviceConfig.playback.format = gDecoder->outputFormat;
            deviceConfig.playback.channels = gDecoder->outputChannels;
            deviceConfig.sampleRate = gDecoder->outputSampleRate;
            deviceConfig.dataCallback = ma_data_callback;
            deviceConfig.pUserData = gDecoder;
            gDevice = new ma_device;
            if(ma_device_init(NULL, &deviceConfig, gDevice) != MA_SUCCESS) {
                ma_decoder_uninit(gDecoder);
                delete gDecoder;
                gDecoder = nullptr;
                delete gDevice;
                gDevice = nullptr;
                return false;
            }
            if(ma_device_start(gDevice) != MA_SUCCESS) {
                ma_device_uninit(gDevice);
                delete gDevice;
                gDevice = nullptr;
                ma_decoder_uninit(gDecoder);
                delete gDecoder;
                gDecoder = nullptr;
                return false;
            }
            isPlaying = true;
            return true;
        };

        while(InternetReadFile(hUrl, buf.get(), BUF_SIZE, &bytesRead) && bytesRead) {
            WriteFile(hFile, buf.get(), bytesRead, &bytesWritten, NULL);
            FlushFileBuffers(hFile);
            total += bytesRead;

            if(!started && total >= START_THRESHOLD) {
                if(startPlayback(tmpPath)) {
                    started = true;
                    GI::DisplayClientMessage("%s[Music]%s Playing (buffered)", MCTF::GOLD,
                                             MCTF::WHITE);
                }
            }
        }

        CloseHandle(hFile);
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hSession);

        if(!started) {
            if(startPlayback(tmpPath)) {
                GI::DisplayClientMessage("%s[Music]%s Playing..", MCTF::GOLD, MCTF::WHITE);
            } else {
                GI::DisplayClientMessage("%s[Music]%s Unable to play file", MCTF::RED, MCTF::WHITE);
            }
        }
    });
}

void NeteaseMusicGUI::searchSongs(const std::string& keyword) {
    GI::DisplayClientMessage("%s[Music]%s Searching \"%s\"...", MCTF::GOLD, MCTF::WHITE,
                             keyword.c_str());

    auto startTime = std::chrono::steady_clock::now();
    std::string url = API_BASE + "/cloudsearch?keywords=" + NetworkUtil::urlEncode(keyword);
    std::string response;
    if(!NetworkUtil::httpGet(url, response)) {
        GI::DisplayClientMessage("%s[Music]%s Search failed - network error", MCTF::RED,
                                 MCTF::WHITE);
        return;
    }

    try {
        auto jsonData = json::parse(response);
        if(!jsonData.contains("result") || !jsonData["result"].contains("songs")) {
            GI::DisplayClientMessage("%s[Music]%s No results found", MCTF::YELLOW, MCTF::WHITE);
            return;
        }

        auto songList = jsonData["result"]["songs"];
        songs.clear();

        for(auto& songData : songList) {
            if(songs.size() >= maxDisplaySongs)
                break;  // performance limit

            Song song;
            // --- PARSE SONG INFO (robust) ---
            song.id = songData["id"].get<int>();
            song.name = songData["name"].get<std::string>();

            // Artist name
            if(songData.contains("ar") && songData["ar"].is_array() && !songData["ar"].empty()) {
                song.artist = songData["ar"][0]["name"].get<std::string>();
            } else if(songData.contains("artists") && songData["artists"].is_array() &&
                      !songData["artists"].empty()) {
                song.artist = songData["artists"][0]["name"].get<std::string>();
            }

            if(songData.contains("al") && songData["al"].contains("picUrl") &&
               !songData["al"]["picUrl"].is_null()) {
                song.albumCover = songData["al"]["picUrl"].get<std::string>();
            } else if(songData.contains("album") && songData["album"].contains("picUrl") &&
                      !songData["album"]["picUrl"].is_null()) {
                song.albumCover = songData["album"]["picUrl"].get<std::string>();
            }

            // VIP status
            if(songData.contains("privilege") && songData["privilege"].contains("fee")) {
                int fee = songData["privilege"]["fee"].get<int>();
                song.vip = (fee == 1);
            }
            // ----------------------------------------------------

            songs.push_back(song);
            if(songs.size() >= 50)
                break;
        }

        selectedSong = 0;
        auto endTime = std::chrono::steady_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        GI::DisplayClientMessage("%s[Music]%s Found %d songs (%lld ms)", MCTF::GOLD, MCTF::WHITE,
                                 (int)songs.size(), duration);
    } catch(...) {
        GI::DisplayClientMessage("%s[Music]%s Search failed - invalid response", MCTF::RED,
                                 MCTF::WHITE);
    }
}

bool NeteaseMusicGUI::shouldRender() const {
    auto clickGui = ModuleManager::getModule<ClickGUI>();
    return clickGui && clickGui->isEnabled();
}

void NeteaseMusicGUI::stopPlayback() {
    if(gDevice) {
        ma_device_stop(gDevice);
        ma_device_uninit(gDevice);
        delete gDevice;
        gDevice = nullptr;
    }
    if(gDecoder) {
        ma_decoder_uninit(gDecoder);
        delete gDecoder;
        gDecoder = nullptr;
    }
    isPlaying = false;
    joinThreads();
}

void NeteaseMusicGUI::joinThreads() {
    if(audioThread.joinable()) {
        try {
            audioThread.join();
        } catch(...) {
        }
    }
    if(searchThread.joinable()) {
        try {
            searchThread.join();
        } catch(...) {
        }
    }
}

void NeteaseMusicGUI::startSearch(const std::string& keyword) {
    if(isSearching)
        return;

    joinThreads();
    isSearching = true;
    searchThread = std::thread([this, keyword]() {
        searchSongs(keyword);
        isSearching = false;
    });
}

void NeteaseMusicGUI::togglePlayPause() {
    if(!gDecoder || !gDevice) {
        if(!songs.empty() && selectedSong < songs.size()) {
            playSong(selectedSong);
        }
        return;
    }

    if(isPlaying && !isPaused) {
        ma_device_stop(gDevice);
        isPaused = true;
    } else {
        ma_device_start(gDevice);
        isPaused = false;
    }
    isPlaying = !isPaused;
}

void NeteaseMusicGUI::nextSong() {
    if(selectedSong + 1 < songs.size()) {
        selectedSong++;
        playSong(selectedSong);
    }
}

void NeteaseMusicGUI::previousSong() {
    if(selectedSong > 0) {
        selectedSong--;
        playSong(selectedSong);
    }
}

void NeteaseMusicGUI::renderUI() {
    if(!shouldRender())
        return;

    float s = size;
    Vec2<float> pos = {panelPos.x * s, panelPos.y * s};
    Vec2<float> size = {panelSize.x * s, panelSize.y * s};

    // Main panel background
    Vec4<float> mainRect = {pos.x, pos.y, pos.x + size.x, pos.y + size.y};
    RenderUtil::fillRoundedRectangle(mainRect, UIColor(25, 25, 25, 240), 12.f * s);

    const float padding = 15.f * s;
    const float headerHeight = 40.f * s;
    const float playerHeight = 50.f * s;

    // Header with search
    Vec4<float> headerRect = {pos.x + padding, pos.y + padding, pos.x + size.x - padding,
                              pos.y + padding + headerHeight};
    RenderUtil::fillRoundedRectangle(headerRect, UIColor(35, 35, 35, 200), 8.f * s);

    // Search box with enhanced visual feedback
    Vec4<float> searchRect = {headerRect.x + 10.f * s, headerRect.y + 6.f * s,
                              headerRect.z - 16.f * s, headerRect.w - 8.f * s};

    // Enhanced background colors for better feedback
    UIColor searchBg;
    UIColor borderColor;

    // Check if click effect should still be visible (300ms duration)
    bool showClickEffect = false;
    if(searchBoxClicked) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - searchClickTime).count();
        if(elapsed < 300) {
            showClickEffect = true;
        } else {
            searchBoxClicked = false;
        }
    }

    if(searchFocused) {
        if(showClickEffect) {
            searchBg = UIColor(80, 80, 80, 255);        // Brighter when clicked
            borderColor = UIColor(150, 200, 255, 255);  // Brighter blue border
        } else {
            searchBg = UIColor(60, 60, 60, 255);
            borderColor = UIColor(100, 150, 255, 200);  // Blue border when focused
        }
    } else {
        searchBg = UIColor(40, 40, 40, 200);
        borderColor = UIColor(80, 80, 80, 150);  // Gray border when not focused
    }

    // Draw background
    RenderUtil::fillRoundedRectangle(searchRect, searchBg, 6.f * s);

    // Draw border for visual feedback
    Vec4<float> borderRect = {searchRect.x - 1.f * s, searchRect.y - 1.f * s,
                              searchRect.z + 1.f * s, searchRect.w + 1.f * s};
    RenderUtil::drawRoundedRectangle(borderRect, borderColor, 6.f * s, 1.5f * s);

    // Search text with better visual states
    std::string displayText;
    UIColor textColor;

    if(isSearching) {
        displayText = "Searching...";
        textColor = UIColor(100, 150, 255, 255);  // Blue when searching
    } else if(searchQuery.empty()) {
        if(searchFocused) {
            displayText = "|";  // Show cursor when focused and empty
            textColor = UIColor(200, 200, 200, 255);
        } else {
            displayText = "Search music...";
            textColor = UIColor(120, 120, 120, 255);  // Dimmer placeholder
        }
    } else {
        displayText = searchQuery;
        if(searchFocused) {
            displayText += "|";  // Show cursor when typing
        }
        textColor = UIColor(220, 220, 220, 255);
    }

    RenderUtil::drawText({searchRect.x + 8.f * s, searchRect.y + 6.f * s}, displayText, textColor,
                         0.75f * s);

    // Song list area
    float listTop = headerRect.w + padding;
    float listBottom = pos.y + size.y - playerHeight - padding * 2;
    Vec4<float> listRect = {pos.x + padding, listTop, pos.x + size.x - padding, listBottom};

    // Song items (simplified list view instead of grid)
    const float thumbSize = 30.f * s;
    const float itemHeight = 40.f * s;
    const float itemSpacing = 2.f * s;
    int visibleItems = (int)((listRect.w - listRect.y) / (itemHeight + itemSpacing));
    int displayCount = std::min((int)songs.size(), visibleItems);

    for(int i = 0; i < displayCount; ++i) {
        float itemY = listRect.y + i * (itemHeight + itemSpacing);
        Vec4<float> itemRect = {listRect.x, itemY, listRect.z, itemY + itemHeight};

        // Item background
        UIColor itemBg =
            (i == selectedSong) ? UIColor(60, 120, 200, 100) : UIColor(40, 40, 40, 150);
        RenderUtil::fillRoundedRectangle(itemRect, itemBg, 6.f * s);

        // Thumbnail
        Vec4<float> thumbRect = {itemRect.x + 4.f * s, itemRect.y + (itemHeight - thumbSize) / 2,
                                 itemRect.x + 4.f * s + thumbSize,
                                 itemRect.y + (itemHeight - thumbSize) / 2 + thumbSize};
        if(!songs[i].albumCover.empty()) {
            RenderUtil::drawImage(thumbRect, songs[i].albumCover);
        } else {
            // GI::DisplayClientMessage("%s[Music]%s No album cover for %s", MCTF::YELLOW,
            // MCTF::WHITE, songs[i].name.c_str());
            RenderUtil::fillRoundedRectangle(thumbRect, UIColor(80, 80, 80, 150),
                                             4.f * s);  // placeholder
        }

        // Song info
        std::string songTitle = songs[i].name;
        bool isVip = songs[i].vip;
        if(isVip)
            songTitle += " [VIP]";

        if(songTitle.length() > 30)
            songTitle = songTitle.substr(0, 27) + "...";

        std::string artistName = songs[i].artist;
        if(artistName.length() > 25)
            artistName = artistName.substr(0, 22) + "...";

        // Song title
        float textStartX = itemRect.x + thumbSize + 10.f * s;
        RenderUtil::drawText({textStartX, itemRect.y + 4.f * s}, songTitle,
                             isVip ? UIColor(255, 215, 0, 255) : UIColor(220, 220, 220, 255),
                             0.7f * s);

        // Artist name
        RenderUtil::drawText({textStartX, itemRect.y + 20.f * s}, artistName,
                             UIColor(160, 160, 160, 255), 0.6f * s);
    }

    // Player controls at bottom
    Vec4<float> playerRect = {pos.x + padding, pos.y + size.y - playerHeight - padding,
                              pos.x + size.x - padding, pos.y + size.y - padding};
    RenderUtil::fillRoundedRectangle(playerRect, UIColor(45, 45, 45, 220), 8.f * s);

    // Current song info with album art
    float infoStartX = playerRect.x + 10.f * s;
    if(!songs.empty() && selectedSong < songs.size()) {
        Vec4<float> coverRect = {playerRect.x + 10.f * s, playerRect.y + 5.f * s,
                                 playerRect.x + 10.f * s + 40.f * s,
                                 playerRect.y + 5.f * s + 40.f * s};
        if(!songs[selectedSong].albumCover.empty()) {
            RenderUtil::drawImage(coverRect, songs[selectedSong].albumCover);
        } else {
            // Game::DisplayClientMessage("%s[Music]%s No album cover for %s", MCTF::YELLOW,
            // MCTF::WHITE, songs[selectedSong].name.c_str());
            RenderUtil::fillRoundedRectangle(coverRect, UIColor(80, 80, 80, 150), 6.f * s);
        }
        infoStartX = coverRect.z + 10.f * s;
    }

    std::string currentSongLine1 = "No song selected";
    std::string currentSongLine2;
    if(!songs.empty() && selectedSong < songs.size()) {
        currentSongLine1 = songs[selectedSong].name;
        if(currentSongLine1.length() > 30)
            currentSongLine1 = currentSongLine1.substr(0, 27) + "...";
        currentSongLine2 = songs[selectedSong].artist;
        if(currentSongLine2.length() > 30)
            currentSongLine2 = currentSongLine2.substr(0, 27) + "...";
    }

    RenderUtil::drawText({infoStartX, playerRect.y + 6.f * s}, currentSongLine1,
                         UIColor(220, 220, 220, 255), 0.7f * s);
    if(!currentSongLine2.empty()) {
        RenderUtil::drawText({infoStartX, playerRect.y + 22.f * s}, currentSongLine2,
                             UIColor(160, 160, 160, 255), 0.6f * s);
    }

    // Control buttons (simplified)
    const float btnSize = 20.f * s;
    const float btnSpacing = 25.f * s;
    float btnY = playerRect.y + (playerHeight - btnSize) / 2;
    float btnStartX = playerRect.z - 120.f * s;

    Vec4<float> prevBtn = {btnStartX, btnY, btnStartX + btnSize, btnY + btnSize};
    Vec4<float> playBtn = {prevBtn.z + btnSpacing, btnY, prevBtn.z + btnSpacing + btnSize,
                           btnY + btnSize};
    Vec4<float> nextBtn = {playBtn.z + btnSpacing, btnY, playBtn.z + btnSpacing + btnSize,
                           btnY + btnSize};

    // Button icons (simplified)
    RenderUtil::drawTriangle(prevBtn, UIColor(200, 200, 200, 255), true, false);

    if(isPlaying && !isPaused) {
        RenderUtil::drawPause(playBtn, UIColor(200, 200, 200, 255));
    } else {
        RenderUtil::drawTriangle(playBtn, UIColor(200, 200, 200, 255), false, true);
    }

    RenderUtil::drawTriangle(nextBtn, UIColor(200, 200, 200, 255), false, false);

    // -------- Volume Slider --------
    float volumeBarWidth = 100.f * s;
    float volumeBarHeight = 6.f * s;
    // 音量条放到“上一首”按钮左侧
    float volumeStartX = prevBtn.x - btnSpacing - volumeBarWidth;
    float volumeY = playerRect.y + (playerHeight - volumeBarHeight) / 2.0f;
    Vec4<float> volumeBarRect = {volumeStartX, volumeY, volumeStartX + volumeBarWidth,
                                 volumeY + volumeBarHeight};

    // Slider background
    RenderUtil::fillRoundedRectangle(volumeBarRect, UIColor(70, 70, 70, 180), volumeBarHeight / 2);

    // Filled portion
    float filledWidth = volumeBarWidth * volume;
    Vec4<float> filledRect = {volumeBarRect.x, volumeBarRect.y, volumeBarRect.x + filledWidth,
                              volumeBarRect.w};
    RenderUtil::fillRoundedRectangle(filledRect, UIColor(100, 180, 255, 255), volumeBarHeight / 2);

    // Knob
    float knobRadius = 4.f * s;
    float knobX = volumeBarRect.x + filledWidth;
    float knobY = volumeBarRect.y + volumeBarHeight / 2.0f;
    RenderUtil::fillCircle({knobX, knobY}, UIColor(220, 220, 220, 255), knobRadius);
}

void NeteaseMusicGUI::Render() {
    renderUI();
}

bool NeteaseMusicGUI::onMouseUpdate(Vec2<float> mousePos, char mouseButton, char isDown) {
    if(!shouldRender())
        return false;

    float s = size;
    Vec2<float> pos = {panelPos.x * s, panelPos.y * s};
    Vec2<float> size = {panelSize.x * s, panelSize.y * s};

    Vec4<float> mainRect = {pos.x, pos.y, pos.x + size.x, pos.y + size.y};
    bool inPanel = mousePos.x >= mainRect.x && mousePos.x <= mainRect.z &&
                   mousePos.y >= mainRect.y && mousePos.y <= mainRect.w;

    if(!inPanel)
        return false;

    const float padding = 15.f * s;
    const float headerHeight = 40.f * s;
    const float playerHeight = 50.f * s;

    // Title bar for dragging (top area)
    Vec4<float> titleRect = {pos.x, pos.y, pos.x + size.x, pos.y + headerHeight + padding};

    if(mouseButton == 1 && isDown) {
        if(!dragging && mousePos.x >= titleRect.x && mousePos.x <= titleRect.z &&
           mousePos.y >= titleRect.y && mousePos.y <= titleRect.w) {
            dragging = true;
            dragOffset = {mousePos.x - pos.x, mousePos.y - pos.y};
        }
    } else if(mouseButton == 0 && dragging) {
        panelPos = {(mousePos.x - dragOffset.x) / s, (mousePos.y - dragOffset.y) / s};
    } else if(mouseButton == 1 && !isDown) {
        dragging = false;
    }

    // Search box interaction
    Vec4<float> headerRect = {pos.x + padding, pos.y + padding, pos.x + size.x - padding,
                              pos.y + padding + headerHeight};
    Vec4<float> searchRect = {headerRect.x + 10.f * s, headerRect.y + 6.f * s,
                              headerRect.z - 16.f * s, headerRect.w - 8.f * s};

    if(mouseButton == 1 && isDown) {
        if(mousePos.x >= searchRect.x && mousePos.x <= searchRect.z && mousePos.y >= searchRect.y &&
           mousePos.y <= searchRect.w) {
            searchFocused = true;
            searchBoxClicked = true;
            searchClickTime = std::chrono::steady_clock::now();

            // Visual feedback - brief highlight effect
            GI::DisplayClientMessage("%s[Music]%s Search box activated", MCTF::BLUE, MCTF::WHITE);
        } else {
            searchFocused = false;
        }
    }

    // Song list interaction
    float listTop = headerRect.w + padding;
    float listBottom = pos.y + size.y - playerHeight - padding * 2;
    Vec4<float> listRect = {pos.x + padding, listTop, pos.x + size.x - padding, listBottom};

    const float itemHeight = 35.f * s;
    const float itemSpacing = 2.f * s;
    int visibleItems = (int)((listRect.w - listRect.y) / (itemHeight + itemSpacing));
    int displayCount = std::min((int)songs.size(), visibleItems);

    if(mouseButton == 1 && isDown) {
        for(int i = 0; i < displayCount; ++i) {
            float itemY = listRect.y + i * (itemHeight + itemSpacing);
            Vec4<float> itemRect = {listRect.x, itemY, listRect.z, itemY + itemHeight};

            if(mousePos.x >= itemRect.x && mousePos.x <= itemRect.z && mousePos.y >= itemRect.y &&
               mousePos.y <= itemRect.w) {
                if(selectedSong == i) {
                    playSong(i);
                } else {
                    selectedSong = i;
                }
                return true;
            }
        }
    }

    // Player controls interaction
    Vec4<float> playerRect = {pos.x + padding, pos.y + size.y - playerHeight - padding,
                              pos.x + size.x - padding, pos.y + size.y - padding};

    const float btnSize = 20.f * s;
    const float btnSpacing = 25.f * s;
    float btnY = playerRect.y + (playerHeight - btnSize) / 2;
    float btnStartX = playerRect.z - 120.f * s;

    Vec4<float> prevBtn = {btnStartX, btnY, btnStartX + btnSize, btnY + btnSize};
    Vec4<float> playBtn = {prevBtn.z + btnSpacing, btnY, prevBtn.z + btnSpacing + btnSize,
                           btnY + btnSize};
    Vec4<float> nextBtn = {playBtn.z + btnSpacing, btnY, playBtn.z + btnSpacing + btnSize,
                           btnY + btnSize};

    // Volume slider geometry (same as in Render())
    float volumeBarWidth = 100.f * s;
    float volumeBarHeight = 6.f * s;
    // 音量条放到“上一首”按钮左侧（与Render保持一致）
    float volumeStartX = prevBtn.x - btnSpacing - volumeBarWidth;
    float volumeY = playerRect.y + (playerHeight - volumeBarHeight) / 2.0f;
    Vec4<float> volumeBarRect = {volumeStartX, volumeY, volumeStartX + volumeBarWidth,
                                 volumeY + volumeBarHeight};

    // 如果正在拖动音量，实时更新（支持移动事件 0 以及持续按下事件 1&isDown）
    if(draggingVolume) {
        if(mouseButton == 0 || (mouseButton == 1 && isDown)) {
            float newVol = (mousePos.x - volumeBarRect.x) / volumeBarWidth;
            if(newVol < 0.f)
                newVol = 0.f;
            if(newVol > 1.f)
                newVol = 1.f;
            volume = newVol;
            gMasterVolume.store(volume);
        }
        if(mouseButton == 1 && !isDown) {
            draggingVolume = false;
        }
    }

    if(mouseButton == 1 && isDown) {
        if(mousePos.x >= playBtn.x && mousePos.x <= playBtn.z && mousePos.y >= playBtn.y &&
           mousePos.y <= playBtn.w) {
            togglePlayPause();
            return true;
        }
        if(mousePos.x >= prevBtn.x && mousePos.x <= prevBtn.z && mousePos.y >= prevBtn.y &&
           mousePos.y <= prevBtn.w) {
            previousSong();
            return true;
        }
        if(mousePos.x >= nextBtn.x && mousePos.x <= nextBtn.z && mousePos.y >= nextBtn.y &&
           mousePos.y <= nextBtn.w) {
            nextSong();
            return true;
        }

        // Volume slider press
        if(mousePos.x >= volumeBarRect.x && mousePos.x <= volumeBarRect.z &&
           mousePos.y >= volumeBarRect.y && mousePos.y <= volumeBarRect.w) {
            draggingVolume = true;
            float newVol = (mousePos.x - volumeBarRect.x) / volumeBarWidth;
            if(newVol < 0.f)
                newVol = 0.f;
            if(newVol > 1.f)
                newVol = 1.f;
            volume = newVol;
            gMasterVolume.store(volume);
            return true;
        }
    }

    // Dragging volume while moving mouse
    if(mouseButton == 0 && draggingVolume) {
        float newVol = (mousePos.x - volumeBarRect.x) / volumeBarWidth;
        if(newVol < 0.f)
            newVol = 0.f;
        if(newVol > 1.f)
            newVol = 1.f;
        volume = newVol;
        gMasterVolume.store(volume);
    }

    // Release volume dragging
    if(mouseButton == 1 && !isDown && draggingVolume) {
        draggingVolume = false;
    }

    return true;
}

void NeteaseMusicGUI::onD2DRender() {
    Render();
}

void NeteaseMusicGUI::onKeyUpdate(int key, bool isDown) {
    if(!shouldRender() || !searchFocused || !isDown)
        return;

    if(key == VK_RETURN) {
        if(!searchQuery.empty()) {
            startSearch(searchQuery);
        }
        searchFocused = false;
        return;
    }

    if(key == VK_ESCAPE) {
        searchFocused = false;
        return;
    }

    if(key == VK_BACK) {
        if(!searchQuery.empty()) {
            searchQuery.pop_back();
        }
        return;
    }

    // Character input
    char ch = 0;
    if(key == VK_SPACE) {
        ch = ' ';
    } else if(key >= '0' && key <= '9') {
        ch = (char)key;
    } else if(key >= 'A' && key <= 'Z') {
        bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        ch = shift ? (char)key : (char)(key + 32);
    } else if(key == VK_OEM_PERIOD) {
        ch = '.';
    } else if(key == VK_OEM_COMMA) {
        ch = ',';
    }

    if(ch != 0 && searchQuery.size() < 100) {
        searchQuery += ch;
    }
}

void NeteaseMusicGUI::savePanelState(nlohmann::json& j) const {
    j["panelPos_x"] = panelPos.x;
    j["panelPos_y"] = panelPos.y;
    j["panelSize_x"] = panelSize.x;
    j["panelSize_y"] = panelSize.y;
}

void NeteaseMusicGUI::loadPanelState(const nlohmann::json& j) {
    if(j.contains("panelPos_x"))
        panelPos.x = j["panelPos_x"];
    if(j.contains("panelPos_y"))
        panelPos.y = j["panelPos_y"];
    if(j.contains("panelSize_x"))
        panelSize.x = j["panelSize_x"];
    if(j.contains("panelSize_y"))
        panelSize.y = j["panelSize_y"];
}

void NeteaseMusicGUI::onDisable() {
    stopPlayback();
    joinThreads();
}