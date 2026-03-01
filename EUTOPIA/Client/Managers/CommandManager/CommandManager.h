#pragma once
#include "Commands/CommandBase.h"



class CommandManager {
public:
	static inline std::vector<CommandBase*> commandList;
	static inline char prefix = '.';

	static void init();
	static void shutdown();
	static void execute(const std::string& message);
};