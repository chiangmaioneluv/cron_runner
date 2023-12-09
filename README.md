This is a simple background task runner!
Task sheduling is stopped, when the destructor of CronRunner class is called.

Example code:

#include <iostream>
#include "cron_runner/cron_runner.h"

void Print() {
    std::cout << "Hello!\n";
}

int main() {
    std::cout << "start\n";
    CronRunner::CronRunner runner(1);
    runner.AddTask(Print);
    runner.Start();


    using namespace std::chrono_literals;
    std::this_thread::sleep_for(130s);
    std::cout << "end\n";
}
