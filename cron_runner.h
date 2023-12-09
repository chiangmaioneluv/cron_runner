#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <thread>
#include <string>

namespace CronRunner {

class Task;

class CronRunner {
public:
    explicit CronRunner(int interval_minutes);
    ~CronRunner();
    CronRunner (const CronRunner&) = delete;
    CronRunner (CronRunner&& other);
    CronRunner& operator= (const CronRunner&) = delete;
    CronRunner& operator= (CronRunner&&) = delete;

    void AddTask(std::function<void()>&& user_function);

    void Start();

private:
    void RunTasks();
    void Sleep();

private:
    std::list<Task> tasks_;
    std::unique_ptr<std::thread> executor_ = nullptr;
    std::atomic<bool> is_running_{false};
    const std::chrono::minutes interval_minutes_;
    std::condition_variable sleeper_condition_variable_;
    std::mutex sleeper_mutex_;
};

}