#pragma once
#include <Windows.h>
#include <xaudio2.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
#ifdef _XBOX
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

class Audio {
   private:
    HRESULT hr;
    IXAudio2* pXAudio2;
    IXAudio2MasteringVoice* pMasterVoice;
    HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
    HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset);
    bool createAudioFolder();
    void copySoundFilesFromResources();

   public:
    Audio();
    int PlayFromMemory(const uint8_t* wavData, uint32_t dataSize, float volume, bool ShouldLoop);

    static bool DownloadIfMissing(const std::string& filename); 

    int Play(string path, float volume = 1, bool ShouldLoop = false);
    string BasePath;
};