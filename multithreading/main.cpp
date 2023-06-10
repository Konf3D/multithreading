#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <future> 
#include <memory>
#include <mutex>
#include<queue>
// Функция для выполнения быстрой сортировки
void quickSort(std::vector<int>& array) {
    if (array.size() <= 1)
        return;

    int pivot = array[array.size() - 1];
    std::vector<int> left, right;

    for (size_t i = 0; i < array.size() - 1; i++) {
        if (array[i] < pivot)
            left.push_back(array[i]);
        else
            right.push_back(array[i]);
    }

    quickSort(left);
    quickSort(right);

    std::copy(left.begin(), left.end(), array.begin());
    array[left.size()] = pivot;
    std::copy(right.begin(), right.end(), array.begin() + left.size() + 1);
}

// Класс задачи сортировки
class SortTask {
public:
    SortTask(std::vector<int>& data, std::shared_ptr<std::promise<void>> promise)
        : data_(data), promise_(std::move(promise)) {}

    void operator()() {
        quickSort(data_);
        promise_->set_value();
    }

private:
    std::vector<int>& data_;
    std::shared_ptr<std::promise<void>> promise_;
};

// Пул потоков
class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads) : threads_(numThreads), stop_(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            threads_[i] = std::thread(&ThreadPool::threadLoop, this);
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_ = true;
            condition_.notify_all();
        }

        for (std::thread& thread : threads_) {
            thread.join();
        }
    }

    template <typename Func>
    std::future<void> submitTask(Func func) {
        std::shared_ptr<std::promise<void>> promise = std::make_shared<std::promise<void>>();
        std::future<void> future = promise->get_future();

        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.emplace([func, promise]() mutable {
                func();
                promise->set_value();
                });
        }

        condition_.notify_one();
        return future;
    }

private:
    void threadLoop() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });

                if (stop_ && tasks_.empty()) {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            task();
        }
    }

    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool stop_;
};

int main() {
    std::vector<int> data = { 9, 5, 3, 7, 2, 8, 1, 6, 4 };

    ThreadPool pool(std::thread::hardware_concurrency());

    std::future<void> future = pool.submitTask(SortTask(data, std::make_shared<std::promise<void>>()));

    future.wait();  // Ждем завершения сортировки

    std::cout << "Sorted array: ";
    for (int num : data) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    return 0;
}
