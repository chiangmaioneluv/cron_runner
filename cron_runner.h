#include <atomic>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <thread>

namespace CronRunner {

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

class CronRunner {
public:
    explicit CronRunner(int period_hours = 1) : period_hours_(period_hours) {
    }
    ~CronRunner() {
        is_running_.store(false);
        if (executor_) {
            executor_->join();
        }
    }
    CronRunner (const CronRunner&) = delete;
    CronRunner (CronRunner&& other) 
    : period_hours_(other.period_hours_), 
      tasks_(std::move(other.tasks_)),
      executor_(std::move(other.executor_)),
      is_running_{other.is_running_.load()} {
    }

    CronRunner& operator= (const CronRunner&) = delete;
    CronRunner& operator= (CronRunner&&) = delete;

    void AddTask(std::function<void()>&& user_function) {
        if (is_running_.load()) {
            return;
        }
        tasks_.emplace_back(std::move(user_function));
    }

    void Start() {
        if (is_running_.load()) {
            return;
        }
        is_running_.store(true);
        executor_ = std::make_unique<std::thread> (
            [&]() {
                using namespace std::chrono_literals;
                while (is_running_.load()) {
                    std::this_thread::sleep_for(period_hours_ * 10s);
                    RunTasks();
                }
            }
        );
    }

private:
    void RunTasks() {
        for (auto& task : tasks_) {
            if (!is_running_.load()) {
                return;
            }
            task.TryRun();
        }
    }

private:
    std::list<Task> tasks_;
    std::unique_ptr<std::thread> executor_ = nullptr;
    std::atomic<bool> is_running_{false};
    int period_hours_ = 1;
};

}