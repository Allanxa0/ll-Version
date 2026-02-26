#pragma once

#include "ll/api/mod/NativeMod.h"

namespace glacie {

class GlacieMod {
public:
    static GlacieMod& getInstance();

    GlacieMod() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    bool load();
    bool enable();
    bool disable();

private:
    ll::mod::NativeMod& mSelf;
};

}#pragma once

#include "ll/api/mod/NativeMod.h"

namespace glacie {

class GlacieMod {
public:
    static GlacieMod& getInstance();

    GlacieMod() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    bool load();
    bool enable();
    bool disable();

private:
    ll::mod::NativeMod& mSelf;
};

}