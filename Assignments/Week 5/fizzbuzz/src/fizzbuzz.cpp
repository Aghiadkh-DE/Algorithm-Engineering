#include "fizzbuzz.hpp"

#include <string>

std::string fizzbuzz(int value) {
    if (value % 15 == 0) {
        return "FizzBuzz";
    }
    if (value % 3 == 0) {
        return "Fizz";
    }
    if (value % 5 == 0) {
        return "Buzz";
    }
    return std::to_string(value);
}
