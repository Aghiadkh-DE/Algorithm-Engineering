#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "fizzbuzz.hpp"

TEST_CASE("fizzbuzz returns numbers for non-multiples", "[fizzbuzz]") {
    REQUIRE(fizzbuzz(1) == "1");
    REQUIRE(fizzbuzz(2) == "2");
    REQUIRE(fizzbuzz(4) == "4");
    REQUIRE(fizzbuzz(7) == "7");
    REQUIRE(fizzbuzz(8) == "8");
    REQUIRE(fizzbuzz(11) == "11");
    REQUIRE(fizzbuzz(13) == "13");
    REQUIRE(fizzbuzz(14) == "14");
    REQUIRE(fizzbuzz(16) == "16");
}

TEST_CASE("fizzbuzz returns Fizz for multiples of 3", "[fizzbuzz]") {
    REQUIRE(fizzbuzz(3) == "Fizz");
    REQUIRE(fizzbuzz(6) == "Fizz");
    REQUIRE(fizzbuzz(9) == "Fizz");
    REQUIRE(fizzbuzz(12) == "Fizz");
}

TEST_CASE("fizzbuzz returns Buzz for multiples of 5", "[fizzbuzz]") {
    REQUIRE(fizzbuzz(5) == "Buzz");
    REQUIRE(fizzbuzz(10) == "Buzz");
    REQUIRE(fizzbuzz(20) == "Buzz");
    REQUIRE(fizzbuzz(25) == "Buzz");
}

TEST_CASE("fizzbuzz returns FizzBuzz for multiples of 15", "[fizzbuzz]") {
    REQUIRE(fizzbuzz(15) == "FizzBuzz");
    REQUIRE(fizzbuzz(30) == "FizzBuzz");
    REQUIRE(fizzbuzz(45) == "FizzBuzz");
    REQUIRE(fizzbuzz(60) == "FizzBuzz");
}

TEST_CASE("fizzbuzz handles zero and negative values", "[fizzbuzz]") {
    REQUIRE(fizzbuzz(0) == "FizzBuzz");
    REQUIRE(fizzbuzz(-3) == "Fizz");
    REQUIRE(fizzbuzz(-5) == "Buzz");
    REQUIRE(fizzbuzz(-15) == "FizzBuzz");
    REQUIRE(fizzbuzz(-1) == "-1");
}
