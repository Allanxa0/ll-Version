#pragma once

#include <string>

struct PluginConfig {
    int    version            = 1;
    int    updateIntervalTicks = 20;
    string overworldName      = " Overworld";
    string netherName         = " Nether";
    string endName            = " The End";
    string unknownName        = " Unknown";
    bool   showCoords         = true;
    bool   showStatus         = true;
    bool   showDeaths         = true;
    bool   showDimPop         = true;
    string barOverlay         = "Progress";
};
