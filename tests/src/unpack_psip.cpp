#include <gtest/gtest.h>
#include "spacker/unpack_psip.hpp"

TEST(UnpackTest, AllOnes) {
    std::vector<uint8_t> sample(8, 1);
    auto packed = spacker::pack_psip(sample.size(), sample.data());

    std::vector<uint8_t> unpacked(8);
    spacker::unpack_psip(packed.size(), packed.data(), unpacked.size(), unpacked.data());
    EXPECT_EQ(unpacked, sample);
}
