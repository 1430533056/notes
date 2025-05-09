#include <iostream>
#include <future>
#include <chrono>
#include <thread>
#include <functional>
#include <vector>
#include <algorithm>

// https://www.cnblogs.com/haippy/p/3280643.html

using namespace std::placeholders;

void packaged_task() {
    auto countdown = [] (int from, int to) {
        for (int i=from; i!=to; --i) {
            std::cout << i << '\n';
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout << "Finished!\n";
        return from - to;
    };
    std::packaged_task<int(int,int)> task(countdown);
    std::future<int> ret = task.get_future();
    std::thread th(std::move(task), 10, 0);
    int value = ret.get();
    std::cout << "The countdown lasted for " << value << " seconds.\n";
    th.join();
}

void promise() {
    auto print_int = [] (std::future<int>& fut) {
        int x = fut.get();
        std::cout << "value: " << x << '\n';
    };
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    std::thread t(print_int, std::ref(fut));
    prom.set_value(10);
    t.join();
}

void async() {
    auto f = [](int a, int b) -> int { return a / b; };
    std::future<int> future = std::async(std::bind(f, _2, _1), 2, 4);
    std::cout << future.get() << "\n";
}

int main () {
    async();
    // promise();
    // packaged_task();
    return 0;
}