#include "cron_runner.h"

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <functional>
#include <list>
#include <memory>
#include <thread>

namespace CronRunner {

using namespace std::chrono_literals;

class Task {
public:
    Task(std::function<void()> user_function) 
    : user_function_(std::move(user_function)) {
    }
    ~Task() {
        if (execution_) {
            execution_->join();
        }
    }
    void TryRun() {
        if (is_running_.load()) {
            return;
        }
        if (execution_) {
            execution_->join();
        }
        try {
            is_running_.store(true);

            execution_ = std::make_unique<std::thread>(
                [&]() {
                        try {
                            user_function_();
                            is_running_.store(false);
                        } catch (...) {

                        }
                        is_running_.store(false);
                    } 
            );
        } catch (...) {
            is_running_.store(false);
        }
    }

private:
    std::function<void()> user_function_;
    std::atomic<bool> is_running_;
    std::unique_ptr<std::thread> execution_ = nullptr;
};


CronRunner::CronRunner(int interval_minutes) : interval_minutes_(interval_minutes * 1min) {
    if (interval_minutes <= 0) {
        throw std::logic_error("Time interval <= 0");
    }
}

CronRunner::~CronRunner() {
    is_running_.store(false);
    sleeper_condition_variable_.notify_all();
    if (executor_) {
        executor_->join();
    }
}

CronRunner::CronRunner(CronRunner&& other) 
: interval_minutes_(other.interval_minutes_), 
    tasks_(std::move(other.tasks_)),
    executor_(std::move(other.executor_)),
    is_running_{other.is_running_.load()} {
}

void CronRunner::AddTask(std::function<void()>&& user_function) {
    if (is_running_.load()) {
        return;
    }
    tasks_.emplace_back(std::move(user_function));
}

void CronRunner::Start() {
    if (is_running_.load()) {
        return;
    }
    is_running_.store(true);
    executor_ = std::make_unique<std::thread> (
        [&]() {
            while (is_running_.load()) {
                Sleep();
                RunTasks();
            }
        }
    );
}

void CronRunner::RunTasks() {
    for (auto& task : tasks_) {
        if (!is_running_.load()) {
            return;
        }
        task.TryRun();
    }
}

void CronRunner::Sleep() {
    auto sleep_end = std::chrono::steady_clock::now() + interval_minutes_;
    std::unique_lock<std::mutex> sleeper_lock(sleeper_mutex_);

    while (is_running_.load() && std::chrono::steady_clock::now() < sleep_end) {
        sleeper_condition_variable_.wait_until(sleeper_lock, sleep_end);
    }
}
}