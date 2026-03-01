#include "ConfigManager.h"
#include "../../Client.h"
#include "../../../Utils/FileUtil.h"

#include "../ModuleManager/ModuleManager.h"
#include "../NotificationManager/NotificationManager.h"

void ConfigManager::init() {
    configsPath = FileUtil::getClientPath() + "Configs\\";
    if(!FileUtil::doesFilePathExist(configsPath))
        FileUtil::createPath(configsPath);

    premadeConfigsPath = configsPath + "PremadeConfigs\\";
    if(!FileUtil::doesFilePathExist(premadeConfigsPath))
        FileUtil::createPath(premadeConfigsPath);
}

bool ConfigManager::doesConfigExist(const std::string& name) {
	std::string path = configsPath + name + configFormat;
	return FileUtil::doesFilePathExist(path);
}

void ConfigManager::createNewConfig(const std::string& name) {
    if(!ModuleManager::hasInitialized) {
        //Client::DisplayClientMessage("Insert your key first! .key [key]", MCTF::RED);
        return;
    }

	if (doesConfigExist(name)) {
		Client::DisplayClientMessage("%sFailed to create config %s%s%s. Config already existed!", MCTF::RED, MCTF::GRAY, name.c_str(), MCTF::RED);
		return;
	}

	if (currentConfig != "NULL") {
		saveConfig();
	}

	currentConfig = name;
	saveConfig();

	Client::DisplayClientMessage("Successfully created config %s%s%s!", MCTF::GRAY, name.c_str(), MCTF::WHITE);
} // s blablaalalla

void ConfigManager::loadConfig(const std::string& name) {
    if(!ModuleManager::hasInitialized) {
        //Client::DisplayClientMessage("Insert your key first! .key [key]", MCTF::RED);
        return;
    }

	if (!doesConfigExist(name)) {
		Client::DisplayClientMessage("%sFailed to load config %s%s%s. Config doesn't exist!", MCTF::RED, MCTF::GRAY, name.c_str(), MCTF::RED);
		return;
	}



	/*
	if (currentConfig == name) {
		return;
	}
	*/

	/*
	if (currentConfig != "NULL") {
		saveConfig();
	}
	*/

	currentConfig = name;

	std::string configPath = configsPath + name + configFormat;

	std::ifstream confFile(configPath.c_str(), std::ifstream::binary);
	currentConfigObj.clear();
	confFile >> currentConfigObj;
	ModuleManager::onLoadConfig((void*)&currentConfigObj);

	Client::DisplayClientMessage("Successfully loaded config %s%s%s!", MCTF::GRAY, name.c_str(), MCTF::WHITE);
}

void ConfigManager::saveConfig() {


    if(currentConfig == "NULL")
        return;

    std::string configPath = configsPath + currentConfig + configFormat;

    ModuleManager::onSaveConfig((void*)&currentConfigObj);


    std::ofstream o(configPath, std::ofstream::binary);
    if(!o.is_open()) {
        Client::DisplayClientMessage("Failed to open config file for saving!", MCTF::RED);
        return;
    }

    o << std::setw(4) << currentConfigObj << std::endl;
    o.flush();
    o.close();
}
