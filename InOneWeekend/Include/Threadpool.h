#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <future>
#include <memory>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool
{
public:
    ThreadPool(size_t count) :stop(false)
    {
        for (size_t i = 0; i < count; ++i)
        {
            workers.emplace_back(
                [this]
                {
                    for (;;)
                    {
                        std::function<void()>task;
                        {
                            std::unique_lock<std::mutex>lock(this->mtx);
                            this->condition.wait(lock,
                                [this] { return this->stop || !this->tasks.empty(); });
                            if (this->stop && this->tasks.empty())
                            {
                                return;
                            }
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                }
                );
        }
    }

    template<typename F, typename ...Args>
    auto submit(F&& f, Args&&...args) -> std::future<decltype(f(args...))>
    {
        //using return_type = typename std::invoke_result<F, Args...>::type;
        using return_type = decltype(f(args...));
        auto task = std::make_shared< std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (stop)
            {
                throw std::runtime_error("submit on a stopped threadpool!\n");
            }
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex>lock(this->mtx);
            this->stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }
private:
    std::vector<std::thread>workers;
    std::queue<std::function<void()>>tasks;
    std::mutex mtx;
    std::condition_variable condition;
    bool stop;
};
