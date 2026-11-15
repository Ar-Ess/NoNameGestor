#pragma once

#include "Framework/Coroutine/YieldInstruction.h"
#include "Framework/Engine/ID.h"

#include <coroutine>
#include <memory>

// Create a function returning Coroutine
// Inside, use the co_yeld keyword followed by the Yeld namespace
class Coroutine
{

    friend class CoroutineManager;
    friend struct Yield::WaitForCoroutine;

public:

    struct promise_type
    {
        Coroutine get_return_object()
        {
            return Coroutine{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        template<typename T>
            requires std::derived_from<T, YieldInstruction>
        auto yield_value(T&& value)
        {
            current = std::make_unique<std::decay_t<T>>(std::forward<T>(value));
            return std::suspend_always{};
        }

        auto yield_value(std::nullptr_t)
        {
            current.reset();
            return std::suspend_always{};
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void return_void() {}
        void unhandled_exception() { std::terminate(); }

        std::unique_ptr<YieldInstruction> current;
    };

    Coroutine(Coroutine&& other) noexcept :
        handle(other.handle)
    {
        other.handle = nullptr;
    }

    Coroutine& operator=(Coroutine&& other) noexcept
    {
        if (this != &other)
        {
            if (handle) handle.destroy();
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    ~Coroutine()
    {
        if (handle)
            handle.destroy();
    }


private:

    Coroutine(std::coroutine_handle<promise_type> h) :
        handle(h)
    {
    }

    bool CanResume(float dt) const
    {
        if (!handle || handle.done())
            return false;

        auto& y = handle.promise().current;

        if (!y)
            return true;

        if (y->IsReady(dt))
        {
            y.reset();
            return true;
        }

        return false;
    }

    bool Resume() const
    {
        if (!handle || handle.done())
            return false;

        handle.resume();
        return !handle.done();
    }

    bool IsDone() const
    {
        return !handle || handle.done();
    }

private:

    std::coroutine_handle<promise_type> handle;

private:

    Coroutine(const Coroutine&) = delete;
    Coroutine& operator=(const Coroutine&) = delete;

};