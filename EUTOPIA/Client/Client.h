#pragma once
#include<Windows.h>
#include <wingdi.h>
#include "../Utils/FileUtil.h"
#include "../Utils/Logger.h"

class Client {
  private:
    static inline bool initialized = false;
  public:
    static inline bool isInitialized() { return initialized; }
    static void DisplayClientMessage(const char* fmt, ...);
    static void DisplayIRCMessage(const char* fmt, ...);
    static void AIMessage(const char* fmt, ...);
    //static void DisplayClientMessage(const char* fmt, ...);
    static void init();
    static void shutdown();
  };