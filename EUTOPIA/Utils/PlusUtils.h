#pragma once
#include <string>

class PlusUtils {
   private:
    static bool checked;
    static bool active;

    static std::string fetchMachineId();
    static std::string fetchPlusList(const std::string& url);
    static bool checkPlus(const std::string& url);

   public:
    static bool isPlusActive();
};
