#include <gtest/gtest.h>
#include "spacker/pack.hpp"

TEST(PackTest, Packing) {
    std::vector<uint8_t> sample { 1, 1, 1, 1, 1, 10 };
    auto packed = spacker::pack(sample.size(), sample.data());
    std::cout << packed.compressed.size() << std::endl;
}
