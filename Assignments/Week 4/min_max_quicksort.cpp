#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <parallel/algorithm>
#include <parallel/settings.h>
#include <sstream>
#include <string>
#include <vector>
#include <omp.h>

inline int64_t average(int64_t a, int64_t b) {
    return (a & b) + ((a ^ b) >> 1);
}

inline int64_t
partition(int64_t *arr, int64_t left, int64_t right, int64_t pivot, int64_t &smallest, int64_t &biggest) {
    int64_t *left_ptr = &arr[left];
    int64_t *right_ptr = &arr[right];
    while (left_ptr < right_ptr) {
        smallest = (*left_ptr < smallest) ? *left_ptr : smallest;
        biggest = (*left_ptr > biggest) ? *left_ptr : biggest;
        if (*left_ptr > pivot) {
            --right_ptr;
            std::swap(*left_ptr, *right_ptr);
        } else {
            ++left_ptr;
        }
    }
    return left_ptr - arr;
}

inline void insertion_sort(int64_t *arr, int64_t left, int64_t right) {
    for (int64_t i = left + 1; i <= right; i++) {
        int64_t key = arr[i];
        int64_t j = i - 1;
        while (j >= left && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

// the core recursive quicksort function
void qs_core(int64_t *arr, int64_t left, int64_t right, const int64_t pivot) {
    if (right - left < 32) {
        insertion_sort(arr, left, right);
        return;
    }

    int64_t smallest = std::numeric_limits<int64_t>::max();
    int64_t biggest = std::numeric_limits<int64_t>::min();
    int64_t bound = partition(arr, left, right + 1, pivot, smallest, biggest);

    if (smallest == biggest)
        return;

#pragma omp task final(bound - left < 10000)
    qs_core(arr, left, bound - 1, average(smallest, pivot));
    qs_core(arr, bound, right, average(pivot, biggest));
}

// wrapper for the quicksort function
void min_max_quicksort(int64_t *arr, int64_t n, int num_threads = omp_get_max_threads()) {
#pragma omp parallel num_threads(num_threads)
#pragma omp single nowait
    qs_core(arr, 0, n - 1, n > 0 ? arr[average(0, n - 1)] : 0);
}

// class for generating random numbers
class Xoroshiro128Plus {
    uint64_t state[2]{};

    static inline uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

public:
    explicit Xoroshiro128Plus(uint64_t seed = 0) {
        state[0] = (12345678901234567 + seed) | 0b1001000010000001000100101000000110010010100000011001001010000001ULL;
        state[1] = (98765432109876543 + seed) | 0b0100000011001100100000011001001010000000100100101000000110010010ULL;
        for(int i = 0; i < 10; i++){operator()();}
    }

    uint64_t operator()() {
        const uint64_t s0 = state[0];
        uint64_t s1 = state[1];
        const uint64_t result = s0 + s1;

        s1 ^= s0;
        state[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);
        state[1] = rotl(s1, 37);
        return result;
    }
};

bool verify_qs_correctness(int64_t size) {
    Xoroshiro128Plus generator(size);
    std::vector<int64_t> data(size);
    for (int64_t i = 0; i < size; ++i) {
        data[i] = generator();
    }
    std::vector<int64_t> data_copy = data;  // Duplicate for std::sort
    min_max_quicksort(&data[0], size);
    std::sort(data_copy.begin(), data_copy.end());
    return data == data_copy;  // check if arrays are equal
}

std::vector<int64_t> parse_int64_list(const std::string &text) {
    std::vector<int64_t> values;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) {
            values.push_back(std::stoll(item));
        }
    }
    return values;
}

std::vector<int> parse_int_list(const std::string &text) {
    std::vector<int> values;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) {
            values.push_back(std::stoi(item));
        }
    }
    return values;
}

std::string default_output_path() {
    std::string source_path = __FILE__;
    size_t pos = source_path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return "benchmark_results.csv";
    }
    return source_path.substr(0, pos + 1) + "benchmark_results.csv";
}

bool has_arg(const std::string &arg, const std::string &name, std::string &value) {
    if (arg == name) {
        return false;
    }
    std::string prefix = name + "=";
    if (arg.rfind(prefix, 0) == 0) {
        value = arg.substr(prefix.size());
        return true;
    }
    return false;
}

double median(std::vector<double> values) {
    if (values.empty()) {
        return 0.0;
    }
    std::sort(values.begin(), values.end());
    size_t mid = values.size() / 2;
    if (values.size() % 2 == 1) {
        return values[mid];
    }
    return 0.5 * (values[mid - 1] + values[mid]);
}

double benchmark_std_sort(const std::vector<int64_t> &base, int trials, bool verify) {
    std::vector<double> times;
    times.reserve(trials);
    std::vector<int64_t> work(base.size());
    for (int i = 0; i < trials; ++i) {
        std::copy(base.begin(), base.end(), work.begin());
        double start = omp_get_wtime();
        std::sort(work.begin(), work.end());
        double end = omp_get_wtime();
        if (verify && !std::is_sorted(work.begin(), work.end())) {
            std::cerr << "std::sort failed to sort data.\n";
            std::exit(2);
        }
        times.push_back(end - start);
    }
    return median(times);
}

double benchmark_min_max(const std::vector<int64_t> &base, int threads, int trials, bool verify) {
    std::vector<double> times;
    times.reserve(trials);
    std::vector<int64_t> work(base.size());
    for (int i = 0; i < trials; ++i) {
        std::copy(base.begin(), base.end(), work.begin());
        double start = omp_get_wtime();
        min_max_quicksort(work.data(), static_cast<int64_t>(work.size()), threads);
        double end = omp_get_wtime();
        if (verify && !std::is_sorted(work.begin(), work.end())) {
            std::cerr << "min_max_quicksort failed to sort data.\n";
            std::exit(2);
        }
        times.push_back(end - start);
    }
    return median(times);
}

double benchmark_gnu_parallel(const std::vector<int64_t> &base, int threads, int trials, bool verify) {
    std::vector<double> times;
    times.reserve(trials);
    std::vector<int64_t> work(base.size());
    for (int i = 0; i < trials; ++i) {
        std::copy(base.begin(), base.end(), work.begin());
        double start = omp_get_wtime();
        __gnu_parallel::sort(work.begin(), work.end(), __gnu_parallel::parallel_tag(threads));
        double end = omp_get_wtime();
        if (verify && !std::is_sorted(work.begin(), work.end())) {
            std::cerr << "__gnu_parallel::sort failed to sort data.\n";
            std::exit(2);
        }
        times.push_back(end - start);
    }
    return median(times);
}

std::vector<int64_t> generate_data(int64_t size, uint64_t seed) {
    std::vector<int64_t> data(size);
    Xoroshiro128Plus generator(seed);
    for (int64_t i = 0; i < size; ++i) {
        data[i] = static_cast<int64_t>(generator());
    }
    return data;
}

int main(int argc, char **argv) {
    // Defaults
    int trials = 3;
    uint64_t seed = 3;
    int64_t thread_size = 10000000;
    std::vector<int64_t> size_sweep = {10000000, 20000000, 30000000, 40000000, 50000000, 75000000, 100000000};
    std::vector<int> thread_sweep;
    std::string output_path = default_output_path();
    bool verify = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        std::string value;
        if (arg == "--help") {
            std::cout << "Usage: min_max_quicksort [options]\n"
                      << "  --sizes=LIST            Comma-separated sizes for size sweep\n"
                      << "  --threads=LIST          Comma-separated thread counts for thread sweep\n"
                      << "  --thread-size=N         Array size for thread sweep (>=1e7 recommended)\n"
                      << "  --trials=N              Trials per algorithm (median)\n"
                      << "  --seed=N                RNG seed\n"
                      << "  --out=FILE              CSV output path\n"
                      << "  --verify                Verify sorting correctness\n";
            return 0;
        }

        if (has_arg(arg, "--sizes", value)) {
            size_sweep = parse_int64_list(value);
        } else if (arg == "--sizes" && i + 1 < argc) {
            size_sweep = parse_int64_list(argv[++i]);
        } else if (has_arg(arg, "--threads", value)) {
            thread_sweep = parse_int_list(value);
        } else if (arg == "--threads" && i + 1 < argc) {
            thread_sweep = parse_int_list(argv[++i]);
        } else if (has_arg(arg, "--thread-size", value)) {
            thread_size = std::stoll(value);
        } else if (arg == "--thread-size" && i + 1 < argc) {
            thread_size = std::stoll(argv[++i]);
        } else if (has_arg(arg, "--trials", value)) {
            trials = std::stoi(value);
        } else if (arg == "--trials" && i + 1 < argc) {
            trials = std::stoi(argv[++i]);
        } else if (has_arg(arg, "--seed", value)) {
            seed = static_cast<uint64_t>(std::stoull(value));
        } else if (arg == "--seed" && i + 1 < argc) {
            seed = static_cast<uint64_t>(std::stoull(argv[++i]));
        } else if (has_arg(arg, "--out", value)) {
            output_path = value;
        } else if (arg == "--out" && i + 1 < argc) {
            output_path = argv[++i];
        } else if (arg == "--verify") {
            verify = true;
        }
    }

    // test correctness of min_max_quicksort on small sizes
    const std::vector<int64_t> verify_sizes = {0, 1, 23, 133, 1777, 57462, 786453};
    for (const auto &size : verify_sizes) {
        if (!verify_qs_correctness(size)) {
            std::cout << "min_max_quicksort is incorrect for size " << size << "!\n";
            return 1;
        }
    }

    omp_set_dynamic(0);

    int logical_threads = omp_get_num_procs();
    if (logical_threads <= 0) {
        logical_threads = omp_get_max_threads();
    }
    int max_threads = omp_get_max_threads();
    int sweep_max_threads = 25;
    if (thread_sweep.empty()) {
        thread_sweep.reserve(sweep_max_threads);
        for (int t = 1; t <= sweep_max_threads; ++t) {
            thread_sweep.push_back(t);
        }
    }

    int size_sweep_threads = logical_threads;

    std::ofstream out(output_path);
    if (!out) {
        std::cerr << "Failed to open output file: " << output_path << "\n";
        return 1;
    }
    out << "mode,size,threads,algo,time_s\n";
    out << std::fixed << std::setprecision(6);

    // Thread sweep
    std::cout << "Thread sweep: size=" << thread_size << ", trials=" << trials << "\n";
    std::vector<int64_t> thread_data = generate_data(thread_size, seed);

    for (int threads : thread_sweep) {
        std::cout << "  threads=" << threads << "\n";
        double std_time_thread = benchmark_std_sort(thread_data, trials, verify);
        out << "thread," << thread_size << "," << threads << ",std_sort," << std_time_thread << "\n";

        double mm_time = benchmark_min_max(thread_data, threads, trials, verify);
        out << "thread," << thread_size << "," << threads << ",min_max," << mm_time << "\n";

        double gnu_time = benchmark_gnu_parallel(thread_data, threads, trials, verify);
        out << "thread," << thread_size << "," << threads << ",gnu_parallel," << gnu_time << "\n";
    }

    // Size sweep
    std::cout << "Size sweep: threads=" << size_sweep_threads << ", trials=" << trials << "\n";
    for (int64_t size : size_sweep) {
        std::cout << "  size=" << size << "\n";
        std::vector<int64_t> size_data = generate_data(size, seed);
        double std_time = benchmark_std_sort(size_data, trials, verify);
        out << "size," << size << "," << 1 << ",std_sort," << std_time << "\n";

        double mm_time = benchmark_min_max(size_data, size_sweep_threads, trials, verify);
        out << "size," << size << "," << size_sweep_threads << ",min_max," << mm_time << "\n";

        double gnu_time = benchmark_gnu_parallel(size_data, size_sweep_threads, trials, verify);
        out << "size," << size << "," << size_sweep_threads << ",gnu_parallel," << gnu_time << "\n";
    }

    std::cout << "Results written to " << output_path << "\n";
    return 0;
}
