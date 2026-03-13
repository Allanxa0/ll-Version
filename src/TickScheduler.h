#pragma once

#include <memory>

#include "ll/api/data/CancellableCallback.h"

using namespace std;

class TickScheduler {
public:
    static TickScheduler& getInstance();

    void start(int intervalTicks);
    void stop();

private:
    void tick();
    void scheduleNext();

    int                                    mIntervalTicks = 20;
    shared_ptr<ll::data::CancellableCallback> mCallback;
};
