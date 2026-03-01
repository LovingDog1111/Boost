#include "ThemeCommand.h"

#include <algorithm>

ThemeCommand::ThemeCommand()
    : CommandBase("theme", "Set client color theme", "<theme> [color]", {"t"}) {}

bool ThemeCommand::execute(const std::vector<std::string>& args) {
    if(args.size() < 2)
        return false;
    
    std::string theme = args[1];
    std::transform(theme.begin(), theme.end(), theme.begin(), ::tolower);
    UIColor color = UIColor(255, 255, 255);

    auto colorsModule = ModuleManager::getModule<Colors>();
    if(!colorsModule)
        return true;

    Client::DisplayClientMessage(("Theme set to " + theme).c_str());
    return true;
}

UIColor ThemeCommand::parseColorName(const std::string& name) {
    std::string color = name;
    std::transform(color.begin(), color.end(), color.begin(), ::tolower);

    if(color == "red")
        return UIColor(255, 0, 0);
    if(color == "green")
        return UIColor(0, 255, 0);
    if(color == "blue")
        return UIColor(0, 0, 255);
    if(color == "yellow")
        return UIColor(255, 255, 0);
    if(color == "cyan")
        return UIColor(0, 255, 255);
    if(color == "magenta")
        return UIColor(255, 0, 255);
    if(color == "white")
        return UIColor(255, 255, 255);
    if(color == "black")
        return UIColor(0, 0, 0);
    return UIColor(255, 255, 255);
}
