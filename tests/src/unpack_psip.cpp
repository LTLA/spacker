#include <gtest/gtest.h>
#include "spacker/pack_psip.hpp"
#include "spacker/unpack_psip.hpp"

#include <cstdint>
#include <limits>
#include <random>

/** Simple tests without rles ***/

template<typename T, int N>
void simple_test(T i) {
    std::vector<T> sample(N, i);
    auto packed = spacker::pack_psip<false>(sample.size(), sample.data());

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

/** The same tests, with rle's ***/

template<typename T, int N>
void rle_test(T i) {
    std::vector<T> sample(N, i);
    auto packed = spacker::pack_psip<true>(sample.size(), sample.data());

    std::vector<T> unpacked(N);
    spacker::unpack_psip(packed.size(), packed.data(), unpacked.size(), unpacked.data());
    EXPECT_EQ(sample, unpacked);
}

TEST(UnpackTest, RleByte) {
    for (int i = 1; i <= std::numeric_limits<uint8_t>::max(); ++i) {
        rle_test<uint8_t, 1>(i);
        rle_test<uint8_t, 10>(i);
        rle_test<uint8_t, 100>(i);
        rle_test<uint8_t, 1000>(i);
    }
}

TEST(UnpackTest, RleShort) {
    // Trying some bigger, juicer numbers.
    std::vector<uint16_t> targets{ 1, 10, 100, 1000, 10000 };
    for (auto t : targets) {
        rle_test<uint16_t, 1>(t);
        rle_test<uint16_t, 10>(t);
        rle_test<uint16_t, 100>(t);
        rle_test<uint16_t, 1000>(t);
    }
}

TEST(UnpackTest, RleWord) {
    // Trying some bigger, juicer numbers.
    std::vector<uint32_t> targets{ 1, 10, 100, 1000, 10000, 100000, 1000000 };
    for (auto t : targets) {
        rle_test<uint32_t, 1>(t);
        rle_test<uint32_t, 10>(t);
        rle_test<uint32_t, 100>(t);
        rle_test<uint32_t, 1000>(t);
    }
}

/** Randomized tests, without rle's ***/

template<typename T>
std::vector<T> randomize(size_t n, T max) {
    std::mt19937_64 rng(n * max);
    std::vector<T> output(n);
    for (auto& o : output) {
        o = rng() % max + 1;
    }
    return output;
}

template<bool rle, typename T>
void compare(const std::vector<T>& input) {
    auto packed = spacker::pack_psip<rle>(input.size(), input.data());
    std::vector<T> unpacked(input.size());
    spacker::unpack_psip(packed.size(), packed.data(), unpacked.size(), unpacked.data());
    EXPECT_EQ(input, unpacked);
}

TEST(UnpackTest, RandomUint8) {
    auto output = randomize<uint8_t>(10, 2);
    compare<false>(output);

    output = randomize<uint8_t>(10, 10);
    compare<false>(output);

    output = randomize<uint8_t>(10, 100);
    compare<false>(output);
}

TEST(UnpackTest, RandomUint16) {
    auto output = randomize<uint16_t>(10, 2);
    compare<false>(output);

    output = randomize<uint16_t>(10, 10);
    compare<false>(output);

    output = randomize<uint16_t>(10, 100);
    compare<false>(output);

    output = randomize<uint16_t>(10, 1000);
    compare<false>(output);

    output = randomize<uint16_t>(10, 10000);
    compare<false>(output);

    output = randomize<uint16_t>(10, 50000);
    compare<false>(output);
}

TEST(UnpackTest, RandomUint32) {
    auto output = randomize<uint32_t>(10, 2);
    compare<false>(output);

    output = randomize<uint32_t>(10, 10);
    compare<false>(output);

    output = randomize<uint32_t>(10, 100);
    compare<false>(output);

    output = randomize<uint32_t>(10, 1000);
    compare<false>(output);

    output = randomize<uint32_t>(10, 10000);
    compare<false>(output);

    output = randomize<uint32_t>(10, 100000);
    compare<false>(output);

    output = randomize<uint32_t>(10, 1000000);
    compare<false>(output);
}

/** Randomized tests, with rle's ***/

template<typename T>
std::vector<T> rle_randomize(size_t n, size_t max_rep, T max_val) {
    std::mt19937_64 rng(n * max_rep * max_val);
    std::vector<T> output;
    for (size_t i = 0; i < n; ++i) {
        size_t num = rng() % max_rep + 1;
        T val = rng() % max_val + 1;
        output.insert(output.end(), num, val);
    }
    return output;
}

TEST(UnpackTest, RleUint8) {
    auto output = rle_randomize<uint8_t>(10, 20, 2);
    compare<true>(output);

    output = rle_randomize<uint8_t>(10, 20, 10);
    compare<true>(output);

    output = rle_randomize<uint8_t>(10, 20, 100);
    compare<true>(output);
}

TEST(UnpackTest, RleUint16) {
    auto output = rle_randomize<uint16_t>(10, 20, 2);
    compare<true>(output);

    output = rle_randomize<uint16_t>(10, 20, 10);
    compare<true>(output);

    output = rle_randomize<uint16_t>(10, 20, 100);
    compare<true>(output);

    output = rle_randomize<uint16_t>(10, 20, 1000);
    compare<true>(output);

    output = rle_randomize<uint16_t>(10, 20, 10000);
    compare<true>(output);

    output = rle_randomize<uint16_t>(10, 20, 50000);
    compare<true>(output);
}

TEST(UnpackTest, RleUint32) {
    auto output = rle_randomize<uint32_t>(10, 20, 2);
    compare<true>(output);

    output = rle_randomize<uint32_t>(10, 20, 10);
    compare<true>(output);

    output = rle_randomize<uint32_t>(10, 20, 100);
    compare<true>(output);

    output = rle_randomize<uint32_t>(10, 20, 1000);
    compare<true>(output);

    output = rle_randomize<uint32_t>(10, 20, 10000);
    compare<true>(output);

    output = rle_randomize<uint32_t>(10, 20, 100000);
    compare<true>(output);

    output = rle_randomize<uint32_t>(10, 20, 1000000);
    compare<true>(output);
}
