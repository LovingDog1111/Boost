#pragma once
#include <string>
#include <memory>
#include "../../ModuleManager/ModuleManager.h"

#include "../../../../Utils/Logger.h"
#include "../../../../SDK/GlobalInstance.h"

class FuncHook {
public:
	const char* name;
	uintptr_t address;
	void* OriginFunc;
	void* func;
	bool enable = false;

	virtual bool enableHook();
	virtual void onHookRequest();

	template <auto T>
    auto getOriginal() {
        return reinterpret_cast<decltype(T)>(OriginFunc);
    }
};
