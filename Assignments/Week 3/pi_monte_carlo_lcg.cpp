#include <iostream>
#include <omp.h>
#include <random>

using namespace std;

static double rnd(uint32_t &seed) {
    seed = (1140671485 * seed + 12820163) % (1 << 24);
    return static_cast<double>(seed) / (1.0 * (1 << 24));
}

int main() {
    constexpr int n = 100000000;
    int counter = 0;

    const double start_time = omp_get_wtime();

    #pragma omp parallel reduction(+ : counter)
    {
        uint32_t seed = 0x9e3779b9u ^ static_cast<uint32_t>(omp_get_thread_num() + 1);

        #pragma omp for
        for (int i = 0; i < n; ++i) {
            auto x = rnd(seed);
            auto y = rnd(seed);
            if (x * x + y * y <= 1.0) {
                ++counter;
            }
        }
    }

    const auto run_time = omp_get_wtime() - start_time;
    const auto pi = 4 * (static_cast<double>(counter) / n);

    cout << "pi: " << pi << endl;
    cout << "run_time: " << run_time << " s" << endl;
    cout << "n: " << n << endl;
}
