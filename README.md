This is a simple background task runner!
How to use:
```
1) Set task run interval in CronRunner constructor(in minutes)
2) Add some user functions with AddTask() method. The tasks will be run in a separate execution thread every X minutes
3) Start sheduling with Start() method.
```
Task sheduling is stopped, when the destructor of CronRunner class is called. If the execution time for some task is longer than the run interval, the task will not be run a second time before the first instance is finished.

Example code:
```c++
#include <iostream>
#include "cron_runner/cron_runner.h"

void Print() {
    std::cout << "Hello!\n";
}

int main() {
    std::cout << "Start\n";
    CronRunner::CronRunner runner(1);
    runner.AddTask(Print);
    runner.Start();

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(130s);
    std::cout << "End\n";
}
```
The Print task will be executed every minute. After 130 seconds, when main function is finished, CronRunner's destructor is called, so the task shedulling is stopped.
Example output:
```
Start
Hello!
Hello!
End
```
