#include <gtest/gtest.h>
#include "spacker/pack_psip.hpp"

TEST(PackTest, MaxValues) {
    auto val = spacker::max_value_psip<int, 0>();
    EXPECT_EQ(val, 1);

    val = spacker::max_value_psip<int, 1>();
    EXPECT_EQ(val, 2);

    val = spacker::max_value_psip<int, 2>();
    EXPECT_EQ(val, 4);

    val = spacker::max_value_psip<int, 3>();
    EXPECT_EQ(val, 20);

    val = spacker::max_value_psip<int, 4>();
    EXPECT_EQ(val, 2068);
}

TEST(PackTest, AllOnes) {
    std::vector<uint8_t> sample(8, 1);
    auto packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 1);
    EXPECT_EQ(packed[0], 0);
}

TEST(PackTest, AllTwos) {
    std::vector<uint8_t> sample(4, 2);
    auto packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 1);
    EXPECT_EQ(packed[0], 0b10101010);
}

TEST(PackTest, Nibble) {
    std::vector<uint8_t> sample{ 3, 4 };
    auto packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 1);
    EXPECT_EQ(packed[0], 0b11001101);
}

TEST(PackTest, NibbleOverflow) {
    std::vector<uint8_t> sample{ 1, 3, 4 };
    auto packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b01100110);
    EXPECT_EQ(packed[1], 0b10000000); // trailing 1 bumped over

    sample[0] = 2;
    packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b10110011);
    EXPECT_EQ(packed[1], 0b01000000); // extra bump
}

TEST(PackTest, Byte) {
    std::vector<uint8_t> sample{ 5 };
    auto packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 1);
    EXPECT_EQ(packed[0], 0b11100000);

    sample[0] = 10;
    packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 1);
    EXPECT_EQ(packed[0], 0b11100101); // 0101 = 5 -> +4+1 = 10

    sample[0] = 20;
    packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 1);
    EXPECT_EQ(packed[0], 0b11101111); // 1111 = 15 -> +4+1 = 10
}

TEST(PackTest, ByteOverflow) {
    std::vector<uint8_t> sample{ 4, 6 };
    auto packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b11011110);
    EXPECT_EQ(packed[1], 0b00010000); // 0001 = 1 -> +4+1 = 6

    sample = std::vector<uint8_t>{ 1, 4, 6 };
    packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b01101111);
    EXPECT_EQ(packed[1], 0b00001000);
}

TEST(PackTest, Short) {
    std::vector<uint16_t> sample{ 22 };
    auto packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b11110000);
    EXPECT_EQ(packed[1], 0b00000001); // 00...001 = 1 -> +20+1 = 22

    sample[0] = 2068;
    packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b11110111);
    EXPECT_EQ(packed[1], 0b11111111); // 111.1111 = 2047 -> +20+1 = 2068
}

TEST(PackTest, ShortOverflow) {
    std::vector<uint16_t> sample{ 1, 22, 2068 };
    auto packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 5);
    EXPECT_EQ(packed[0], 0b01111000);
    EXPECT_EQ(packed[1], 0b00000000);
    EXPECT_EQ(packed[2], 0b11111011);
    EXPECT_EQ(packed[3], 0b11111111);
    EXPECT_EQ(packed[4], 0b10000000);

    // Triggering the other overflow-handling clause.
    sample = std::vector<uint16_t>{ 4, 22, 2068 };
    packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 5);
    EXPECT_EQ(packed[0], 0b11011111);
    EXPECT_EQ(packed[1], 0b00000000);
    EXPECT_EQ(packed[2], 0b00011111);
    EXPECT_EQ(packed[3], 0b01111111);
    EXPECT_EQ(packed[4], 0b11110000);
}

TEST(PackTest, Word) {
    std::vector<uint32_t> sample{ spacker::min_value_psip<uint32_t, 5>() + 1 };
    auto packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 4);
    EXPECT_EQ(packed[0], 0b11111000);
    EXPECT_EQ(packed[1], 0);
    EXPECT_EQ(packed[2], 0);
    EXPECT_EQ(packed[3], 1);

    sample = std::vector<uint32_t>{ spacker::max_value_psip<uint32_t, 5>() };
    packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 4);
    EXPECT_EQ(packed[0], 0b11111011);
    EXPECT_EQ(packed[1], 0b11111111);
    EXPECT_EQ(packed[2], 0b11111111);
    EXPECT_EQ(packed[3], 0b11111111);
}

TEST(PackTest, WordOverflow) {
    std::vector<uint32_t> sample{ 0, spacker::min_value_psip<uint32_t, 5>() + 1, 0, spacker::max_value_psip<uint32_t, 5>() };
    auto packed = spacker::pack_psip(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 9);
    EXPECT_EQ(packed[0], 0b01111100);
    EXPECT_EQ(packed[1], 0);
    EXPECT_EQ(packed[2], 0);
    EXPECT_EQ(packed[3], 0);
    EXPECT_EQ(packed[4], 0b10111110);
    EXPECT_EQ(packed[5], 0b11111111);
    EXPECT_EQ(packed[6], 0b11111111);
    EXPECT_EQ(packed[7], 0b11111111);
    EXPECT_EQ(packed[8], 0b11000000);
}
