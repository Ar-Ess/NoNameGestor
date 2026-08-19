#pragma once

#include "Framework/Utils/PerfTimer.h"
#include <type_traits>
#include <concepts>

class Coroutine;

struct YieldInstruction
{
    virtual ~YieldInstruction() = default;
    virtual bool IsReady(float dt) = 0;

};

namespace Yield
{
    struct WaitForSeconds : public YieldInstruction
    {
        float remaining;

        WaitForSeconds(float seconds) : remaining(seconds) {}

        bool IsReady(float dt) override;
    };

    struct WaitForSecondsRealtime : public YieldInstruction
    {
        float time;
        PerfTimer timer;

        WaitForSecondsRealtime(float seconds) : time(seconds) { timer.Start(); }

        bool IsReady(float dt) override;
    };

    struct WaitForFrames : public YieldInstruction
    {
        int frames;

        WaitForFrames(int f) : frames(f) {}

        bool IsReady(float dt) override;
    };

    template<typename Func>
        requires std::is_invocable_r_v<bool, Func>
    struct WaitUntil : public YieldInstruction
    {
        Func func;

        WaitUntil(Func f) : func(f) {}

        bool IsReady(float dt) override
        {
            return func();
        }
    };

    template<typename Func>
        requires std::is_invocable_r_v<bool, Func>
    struct WaitWhile : public YieldInstruction
    {
        Func func;

        WaitWhile(Func f) : func(f) {}

        bool IsReady(float dt) override
        {
            return !func();
        }
    };

    struct WaitForCoroutine : public YieldInstruction
    {
        Coroutine* target;

        WaitForCoroutine(Coroutine* c) : target(c) {}

        bool IsReady(float dt) override;
    };

    struct WaitForNextFrame : public YieldInstruction
    {
        bool first = true;

        bool IsReady(float dt) override;
    };
}