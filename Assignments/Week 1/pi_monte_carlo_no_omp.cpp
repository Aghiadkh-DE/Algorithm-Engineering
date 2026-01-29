#include <iostream>
#include <random>
#include <chrono>
#include <omp.h>

using namespace std;

int main() {
  unsigned int seed = 0;
  default_random_engine re{seed};
  uniform_real_distribution<double> zero_to_one{0.0, 1.0};

  int n = 100000000; // number of points to generate
  int counter = 0; // counter for points lying in the first quadrant of a unit circle
  auto start_time = chrono::high_resolution_clock::now();

  // compute n points and test if they lie within the first quadrant of a unit circle
  for (int i = 0; i < n; ++i) {
    auto x = zero_to_one(re); // generate random number between 0.0 and 1.0
    auto y = zero_to_one(re); // generate random number between 0.0 and 1.0
    if (x * x + y * y <= 1.0) { // if the point lies in the first quadrant of a unit circle
      ++counter;
    }
  }

  auto end_time = chrono::high_resolution_clock::now();
  chrono::duration<double> diff = end_time - start_time;
  const auto run_time = diff.count();
  const auto pi = 4 * (static_cast<double>(counter) / n);

  cout << "pi: " << pi << endl;
  cout << "run_time: " << run_time << " s" << endl;
  cout << "n: " << n << endl;
}
