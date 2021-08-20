#include <gtest/gtest.h>
#include "spacker/unpack_psip.hpp"

#include <cstdint>
#include <limits>
#include <random>

template<typename T, int N, int J = 1>
void simple_test(T i) {
    std::vector<T> sample(N, i);
    auto packed = spacker::pack_psip(sample.size(), sample.data());

    std::vector<T> unpacked(N);
    spacker::unpack_psip(packed.size(), packed.data(), unpacked.size(), unpacked.data());
    EXPECT_EQ(sample, unpacked);
}

TEST(UnpackTest, BasicByte) {
    for (int i = 1; i <= std::numeric_limits<uint8_t>::max(); ++i) {
        simple_test<uint8_t, 1>(i);
        simple_test<uint8_t, 4>(i);
        simple_test<uint8_t, 8>(i);
        simple_test<uint8_t, 12>(i);
        simple_test<uint8_t, 16>(i);
    }
}

TEST(UnpackTest, BasicShort) {
    // Trying some bigger, juicer numbers.
    std::vector<uint16_t> targets{ 1, 10, 100, 1000, 10000 };
    for (auto t : targets) {
        simple_test<uint16_t, 1>(t);
        simple_test<uint16_t, 4>(t);
        simple_test<uint16_t, 8>(t);
        simple_test<uint16_t, 12>(t);
        simple_test<uint16_t, 16>(t);
    }
}

TEST(UnpackTest, BasicWord) {
    // Trying some bigger, juicer numbers.
    std::vector<uint32_t> targets{ 1, 10, 100, 1000, 10000, 100000, 1000000 };
    for (auto t : targets) {
        simple_test<uint32_t, 1>(t);
        simple_test<uint32_t, 4>(t);
        simple_test<uint32_t, 8>(t);
        simple_test<uint32_t, 12>(t);
        simple_test<uint32_t, 32>(t);
    }
}

template<typename T>
std::vector<T> randomize(size_t n, T max) {
    std::mt19937_64 rng(n * max);
    std::vector<T> output(n);
    for (auto& o : output) {
        o = rng() % max + 1;
    }
    return output;
}

template<typename T>
void compare(const std::vector<T>& input) {
    auto packed = spacker::pack_psip(input.size(), input.data());
    std::vector<T> unpacked(input.size());
    spacker::unpack_psip(packed.size(), packed.data(), unpacked.size(), unpacked.data());
    EXPECT_EQ(input, unpacked);
}

TEST(UnpackTest, RandomUint8) {
    auto output = randomize<uint8_t>(10, 2);
    compare(output);

    output = randomize<uint8_t>(10, 10);
    compare(output);

    output = randomize<uint8_t>(10, 100);
    compare(output);
}

TEST(UnpackTest, RandomUint16) {
    auto output = randomize<uint16_t>(10, 2);
    compare(output);

    output = randomize<uint16_t>(10, 10);
    compare(output);

    output = randomize<uint16_t>(10, 100);
    compare(output);

    output = randomize<uint16_t>(10, 1000);
    compare(output);

    output = randomize<uint16_t>(10, 10000);
    compare(output);

    output = randomize<uint16_t>(10, 50000);
    compare(output);
}

TEST(UnpackTest, RandomUint32) {
    auto output = randomize<uint32_t>(10, 2);
    compare(output);

    output = randomize<uint32_t>(10, 10);
    compare(output);

    output = randomize<uint32_t>(10, 100);
    compare(output);

    output = randomize<uint32_t>(10, 1000);
    compare(output);

    output = randomize<uint32_t>(10, 10000);
    compare(output);

    output = randomize<uint32_t>(10, 100000);
    compare(output);

    output = randomize<uint32_t>(10, 1000000);
    compare(output);
}
