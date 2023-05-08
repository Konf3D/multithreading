#include <iostream>
#include <vector>
#include <thread>
#include <numeric>
#include <chrono>
template<typename T>
T calculateSum(const std::vector<T>& v, const size_t m) {
    const size_t n = v.size();
    std::vector<std::thread> threads(m);
    std::vector<T> sums(m);
    const size_t chunkSize = n / m;
    for (size_t i = 0; i < m; i++) {
        threads[i] = std::thread([=, &sums]() {
            const size_t start = i * chunkSize;
            const size_t end = (i == m - 1) ? n : (i + 1) * chunkSize;
            sums[i] = std::accumulate(v.begin() + start, v.begin() + end, static_cast<T>(0));
            });
    }
    for (size_t i = 0; i < m; i++) {
        threads[i].join();
    }
    return std::accumulate(sums.begin(), sums.end(), static_cast<T>(0));
}
int main()
{
    // При использовании бОльшего количества потоков скорость выполнения задачи падает. Мб компилятор добавляет векторизацию.
    // Тесты выполнены на 6-ядерном процессоре.
    for (int n = 100; n <= 100000001; n = n * 100)
    {
        std::cout << "N = " << n << std::endl;
        std::vector<int> v(n);
        for (int i = 0; i < n; ++i) {
            v[i] = rand() % 10000 - 5000;
        }
        for (int m = 16; m > 0; m /= 2)
        {
            const auto start = std::chrono::high_resolution_clock::now();
            const int sum = calculateSum<int>(v, m);
            const auto end = std::chrono::high_resolution_clock::now();
            const std::string time = " miliseconds";
            const auto duration = std::chrono::duration_cast<std::chrono::milliseconds > (end - start);
            std::cout << "Threads: " << m << std::endl << "Duration: " << duration.count() << time << std::endl << "Sum: " << sum << std::endl << std::endl;
        }

    }
    return 0;
}