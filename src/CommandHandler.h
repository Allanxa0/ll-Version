#pragma once

#include "ll/api/mod/NativeMod.h"

class CommandHandler {
public:
    static CommandHandler& getInstance();

    void registerAll(ll::mod::NativeMod& mod);
};
