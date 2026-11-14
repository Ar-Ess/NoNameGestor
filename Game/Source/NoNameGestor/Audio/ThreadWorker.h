#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <atomic>
#include <functional>

template<class In, class Out>
class ThreadWorker
{
public:

    ThreadWorker(std::function<Out(In&)> func) : process(func), thread([this] { Run(); })
    { }

    ~ThreadWorker()
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_one();     // desperta el fil perquè vegi stop
        thread.join();       // espera que acabi
    }

    void Push(In job)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            jobs.push_back(std::move(job));
        }
        cv.notify_one();
    }

    bool TryPop(Out& result)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (results.empty()) return false;
        result = std::move(results.front());
        results.pop_front();
        return true;
    }

private:

    void Run()
    {
        for (;;)
        {
            In job;
            {
                std::unique_lock<std::mutex> l(mtx);
                cv.wait(l, [this] { return stop || !jobs.empty(); });
                if (stop) return;
                job = std::move(jobs.front());
                jobs.pop_front();
            }
            Out result = process(job);          // feina llarga, sense mutex
            {
                std::lock_guard<std::mutex> l(mtx);
                results.push_back(std::move(result));
            }
        }
    }

    std::function<Out(In&)> process;
    std::mutex mtx;
    std::condition_variable cv;
    std::deque<In> jobs;
    std::deque<Out> results;
    bool stop = false;
    std::thread thread;    // l'ÚLTIM membre: es declara després de tot el que usa
};

