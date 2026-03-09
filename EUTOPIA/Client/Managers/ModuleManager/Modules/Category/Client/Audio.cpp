#include "Audio.h"

#include <windows.h>
#include <winhttp.h>

#include <filesystem>
#include <fstream>

#include "FileUtil.h"

bool Audio::DownloadIfMissing(const std::string &filename) {
    std::string fullPath = FileUtil::getClientPath() + "Audios\\" + filename;

    if(std::filesystem::exists(fullPath)) {
        std::cout << "Audio already exists: " << fullPath << std::endl;
        return true;
    }

    std::wstring host = L"127.0.0.1";
    std::wstring path = L"/get_audio?file=" + std::wstring(filename.begin(), filename.end());

    HINTERNET hSession = WinHttpOpen(L"AudioDownloader/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

    if(!hSession)
        return false;

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), 5000, 0);
    if(!hConnect) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL,
                                            WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

    if(!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    bool success = false;

    if(WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0,
                          0) &&
       WinHttpReceiveResponse(hRequest, NULL)) {
        std::ofstream out(fullPath, std::ios::binary);
        if(!out.is_open()) {
            std::cout << "Failed to create file: " << fullPath << std::endl;
        } else {
            DWORD bytesAvailable = 0;
            do {
                WinHttpQueryDataAvailable(hRequest, &bytesAvailable);
                if(!bytesAvailable)
                    break;

                std::vector<char> buffer(bytesAvailable);
                DWORD bytesRead = 0;

                WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead);
                out.write(buffer.data(), bytesRead);

            } while(bytesAvailable > 0);

            out.close();
            success = true;
            std::cout << "Downloaded audio: " << fullPath << std::endl;
        }
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return success;
}

Audio::Audio() {
    BasePath = FileUtil::getClientPath() + "Audios\\";

    std::cout << "Audio base path: " << BasePath << std::endl;

    createAudioFolder();

    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if(FAILED(hr)) {
        std::cout << "CoInitializeEx failed: " << hr << std::endl;
    }

    pXAudio2 = nullptr;
    if(FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
        std::cout << "XAudio2Create failed: " << hr << std::endl;
    }

    pMasterVoice = nullptr;
    if(FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasterVoice))) {
        std::cout << "CreateMasteringVoice failed: " << hr << std::endl;
    }
}

bool Audio::createAudioFolder() {
    if(!std::filesystem::exists(BasePath)) {
        if(std::filesystem::create_directories(BasePath)) {
            std::cout << "Created audio directory: " << BasePath << std::endl;
            return true;
        } else {
            std::cout << "Failed to create audio directory: " << BasePath << std::endl;
            return false;
        }
    }
    return true;
}

int Audio::Play(string path, float volume, bool ShouldLoop) {
    if(!path.empty() && path[0] == '\\') {
        path = path.substr(1);
    }

    std::cout << "Audio::Play attempting to play: " << path << std::endl;

    std::vector<std::string> possiblePaths = {BasePath + path, "audio\\" + path,
                                              FileUtil::getClientPath() + path, path};

    HANDLE hFile = INVALID_HANDLE_VALUE;
    std::string finalPath;

    for(const auto &tryPath : possiblePaths) {
        std::wstring widePath(tryPath.begin(), tryPath.end());
        hFile = CreateFileW(widePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0,
                            NULL);

        if(hFile != INVALID_HANDLE_VALUE) {
            finalPath = tryPath;
            std::cout << "Successfully opened audio file: " << tryPath << std::endl;
            break;
        } else {
            std::cout << "Failed to open: " << tryPath << " (Error: " << GetLastError() << ")"
                      << std::endl;
        }
    }

    if(INVALID_HANDLE_VALUE == hFile) {
        std::cout << "All path attempts failed for: " << path << std::endl;
        std::cout << "Tried paths:" << std::endl;
        for(const auto &p : possiblePaths) {
            std::cout << "  " << p << std::endl;
        }
        return -1;
    }

    WAVEFORMATEXTENSIBLE wfx = {0};
    XAUDIO2_BUFFER buffer = {0};

    if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN)) {
        DWORD error = GetLastError();
        std::cout << "SetFilePointer failed: " << error << std::endl;
        CloseHandle(hFile);
        return HRESULT_FROM_WIN32(error);
    }

    DWORD dwChunkSize;
    DWORD dwChunkPosition;

    if(FAILED(FindChunk(hFile, 'FFIR', dwChunkSize, dwChunkPosition))) {
        std::cout << "FindChunk RIFF failed" << std::endl;
        CloseHandle(hFile);
        return S_FALSE;
    }

    DWORD filetype;
    ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
    if(filetype != 'EVAW') {
        std::cout << "Not a WAV file. File type: " << filetype << std::endl;
        CloseHandle(hFile);
        return S_FALSE;
    }

    if(FAILED(FindChunk(hFile, ' tmf', dwChunkSize, dwChunkPosition))) {
        std::cout << "FindChunk fmt failed" << std::endl;
        CloseHandle(hFile);
        return S_FALSE;
    }
    ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

    if(FAILED(FindChunk(hFile, 'atad', dwChunkSize, dwChunkPosition))) {
        std::cout << "FindChunk data failed" << std::endl;
        CloseHandle(hFile);
        return S_FALSE;
    }

    BYTE *pDataBuffer = new BYTE[dwChunkSize];
    ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

    buffer.AudioBytes = dwChunkSize;
    buffer.pAudioData = pDataBuffer;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    if(ShouldLoop)
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

    IXAudio2SourceVoice *pSourceVoice;
    if(FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX *)&wfx))) {
        std::cout << "CreateSourceVoice failed: " << hr << std::endl;
        delete[] pDataBuffer;
        CloseHandle(hFile);
        return hr;
    }

    if(FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer))) {
        std::cout << "SubmitSourceBuffer failed: " << hr << std::endl;
        delete[] pDataBuffer;
        CloseHandle(hFile);
        return hr;
    }

    pSourceVoice->SetVolume(volume);
    if(FAILED(hr = pSourceVoice->Start(0))) {
        std::cout << "Start failed: " << hr << std::endl;
        delete[] pDataBuffer;
        CloseHandle(hFile);
        return hr;
    }

    CloseHandle(hFile);
    std::cout << "Audio played successfully: " << finalPath << " (Volume: " << volume << ")"
              << std::endl;
    return 0;
}

HRESULT Audio::FindChunk(HANDLE hFile, DWORD fourcc, DWORD &dwChunkSize,
                         DWORD &dwChunkDataPosition) {
    HRESULT hr = S_OK;
    if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwChunkType;
    DWORD dwChunkDataSize;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType;
    DWORD bytesRead = 0;
    DWORD dwOffset = 0;

    while(hr == S_OK) {
        DWORD dwRead;
        if(0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        if(0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        switch(dwChunkType) {
            case 'FFIR':
                dwRIFFDataSize = dwChunkDataSize;
                dwChunkDataSize = 4;
                if(0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
                    hr = HRESULT_FROM_WIN32(GetLastError());
                break;

            default:
                if(INVALID_SET_FILE_POINTER ==
                   SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
                    return HRESULT_FROM_WIN32(GetLastError());
        }

        dwOffset += sizeof(DWORD) * 2;

        if(dwChunkType == fourcc) {
            dwChunkSize = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }

        dwOffset += dwChunkDataSize;

        if(bytesRead >= dwRIFFDataSize)
            return S_FALSE;
    }

    return S_OK;
}
int Audio::PlayFromMemory(const uint8_t *wavData, uint32_t dataSize, float volume,
                          bool ShouldLoop) {
    if(!wavData || dataSize < 44) {
        std::cout << "Invalid WAV data or size too small" << std::endl;
        return -1;
    }

    const BYTE *data = reinterpret_cast<const BYTE *>(wavData);

    if(memcmp(data, "RIFF", 4) != 0) {
        std::cout << "Invalid WAV: No RIFF header" << std::endl;
        return -1;
    }

    if(memcmp(data + 8, "WAVE", 4) != 0) {
        std::cout << "Invalid WAV: No WAVE header" << std::endl;
        return -1;
    }

    DWORD fmtOffset = 12;
    while(fmtOffset + 8 < dataSize) {
        if(memcmp(data + fmtOffset, "fmt ", 4) == 0) {
            break;
        }
        DWORD chunkSize = *(DWORD *)(data + fmtOffset + 4);
        fmtOffset += 8 + chunkSize;
    }

    if(fmtOffset + 8 >= dataSize) {
        std::cout << "Invalid WAV: No fmt chunk found" << std::endl;
        return -1;
    }

    WAVEFORMATEX wfx = {0};
    DWORD fmtSize = *(DWORD *)(data + fmtOffset + 4);
    if(fmtSize >= 16) {
        memcpy(&wfx, data + fmtOffset + 8, 16);
    }

    // Find data chunk
    DWORD dataOffset = fmtOffset + 8 + fmtSize;
    while(dataOffset + 8 < dataSize) {
        if(memcmp(data + dataOffset, "data", 4) == 0) {
            break;
        }
        DWORD chunkSize = *(DWORD *)(data + dataOffset + 4);
        dataOffset += 8 + chunkSize;
    }

    if(dataOffset + 8 >= dataSize) {
        std::cout << "Invalid WAV: No data chunk found" << std::endl;
        return -1;
    }

    DWORD audioDataSize = *(DWORD *)(data + dataOffset + 4);
    DWORD audioDataOffset = dataOffset + 8;

    if(audioDataOffset + audioDataSize > dataSize) {
        audioDataSize = dataSize - audioDataOffset;
    }

    XAUDIO2_BUFFER buffer = {0};
    buffer.AudioBytes = audioDataSize;
    buffer.pAudioData = data + audioDataOffset;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    if(ShouldLoop) {
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    }

    IXAudio2SourceVoice *pSourceVoice;
    HRESULT hr = pXAudio2->CreateSourceVoice(&pSourceVoice, &wfx);
    if(FAILED(hr)) {
        std::cout << "CreateSourceVoice failed: " << hr << std::endl;
        return hr;
    }

    hr = pSourceVoice->SubmitSourceBuffer(&buffer);
    if(FAILED(hr)) {
        std::cout << "SubmitSourceBuffer failed: " << hr << std::endl;
        pSourceVoice->DestroyVoice();
        return hr;
    }

    pSourceVoice->SetVolume(volume);
    hr = pSourceVoice->Start(0);
    if(FAILED(hr)) {
        std::cout << "Start failed: " << hr << std::endl;
        pSourceVoice->DestroyVoice();
        return hr;
    }

    std::cout << "Audio played successfully from memory" << std::endl;
    return 0;
}
HRESULT Audio::ReadChunkData(HANDLE hFile, void *buffer, DWORD buffersize, DWORD bufferoffset) {
    HRESULT hr = S_OK;
    if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());
    DWORD dwRead;
    if(0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
        hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}
