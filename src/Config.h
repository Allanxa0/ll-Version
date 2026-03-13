#pragma once

#include <string>

struct PluginConfig {
    int         version              = 1;
    int         updateIntervalTicks  = 20;
    std::string overworldName        = " Overworld";
    std::string netherName           = " Nether";
    std::string endName              = " The End";
    std::string unknownName          = " Unknown";
    bool        showCoords           = true;
    bool        showStatus           = true;
    bool        showDeaths           = true;
    bool        showDimPop           = true;
    std::string barOverlay           = "Progress";
};
