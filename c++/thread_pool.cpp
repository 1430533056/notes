#include <mutex>
#include <queue>
#include <functional>
#include <future>
#include <thread>
#include <utility>
#include <vector>
#include <iostream>

template <typename T>
class SafeQueue
{
private:
  std::queue<T> m_queue;
  std::mutex m_mutex;

public:
  SafeQueue() {}
  SafeQueue(SafeQueue &&other) {}
  ~SafeQueue() {}

  bool empty() {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_queue.empty();
  }

  int size() {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_queue.size();
  }

  void enqueue(T &t) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.emplace(t);
  }

  bool dequeue(T &t) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_queue.empty())
        return false;
    t = std::move(m_queue.front());
    m_queue.pop();
    return true;
  }
};

class ThreadPool {
private:
  class ThreadWorker {
    private:
      int tid;
      ThreadPool *m_pool;
    public:
      ThreadWorker(ThreadPool *pool, int id) : m_pool(pool), tid(id) {};

      void operator()() {
        std::function<void()> func;
        bool dequeued = false;
        while (!m_pool->stop) {
          {
            std::unique_lock<std::mutex> lock(m_pool->mutex);
            if (m_pool->m_queue.empty()) {
              // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
              // m_pool->variable.wait(lock);
              m_pool->variable.wait(lock, [this](){ return !this->m_pool->m_queue.empty() || this->m_pool->stop;});
            }
            dequeued = m_pool->m_queue.dequeue(func);
          }
          if (dequeued) {
            func();
          }
        }
      }
  };
  bool stop;
  SafeQueue<std::function<void()>> m_queue;
  std::vector<std::thread> m_threads;
  std::mutex mutex;
  std::condition_variable variable;

public:
  ThreadPool(int n_threads = 4) {
    stop = false;
    m_threads = std::vector<std::thread>(n_threads);
    for (int i = 0; i < n_threads; i++) {
      m_threads.at(i) = std::thread(ThreadWorker(this, i));
    }
  }
  ~ThreadPool() {
    stop = true;
    variable.notify_all();
    for (int i = 0; i < m_threads.size(); i++) {
      if (m_threads[i].joinable()) {
        m_threads.at(i).join();
      }
    }
  }

  template<typename F, typename... Args>
  auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
    using Ret = decltype(f(args...));
    std::function<Ret()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    auto task_ptr = std::make_shared<std::packaged_task<Ret()>>(func);
    std::function<void()> wrapper_func = [task_ptr] () {
      (*task_ptr)();
    };
    m_queue.enqueue(wrapper_func);
    variable.notify_one();
    return task_ptr->get_future();
  }
};

void simulate_hard_computation() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void multiply(const int a, const int b) {
    simulate_hard_computation();
    const int res = a * b;
    std::cout << a << " * " << b << " = " << res << std::endl;
}

void multiply_output(int &out, const int a, const int b) {
    simulate_hard_computation();
    out = a * b;
    std::cout << a << " * " << b << " = " << out << std::endl;
}

int multiply_return(const int a, const int b)
{
    simulate_hard_computation();
    const int res = a * b;
    std::cout << a << " * " << b << " = " << res << std::endl;
    return res;
}

void example()
{
    ThreadPool pool(1);

    for (int i = 1; i <= 2; ++i) {
        for (int j = 1; j <= 3; ++j) {
            pool.submit(multiply, i, j);
        }
    }
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    int output_ref;
    auto future1 = pool.submit(multiply_output, std::ref(output_ref), 5, 6);
    future1.get();
    std::cout << "Last operation result is equals to " << output_ref << std::endl;

    auto future2 = pool.submit(multiply_return, 5, 3);
    int res = future2.get();
    std::cout << "Last operation result is equals to " << res << std::endl;
}

int main() {

  example();
  return 0;
}

