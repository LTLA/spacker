#include <gtest/gtest.h>
#include "spacker/pack_psip.hpp"
#include "spacker/Multiplier.hpp"

TEST(PackMultiplierTest, MaxValues) {
    auto val = spacker::max<uint32_t, spacker::Multiplier<>, 0>();
    EXPECT_EQ(val, 8);

    val = spacker::max<uint32_t, spacker::Multiplier<>, 1>();
    EXPECT_EQ(val, 64 + 8);

    val = spacker::max<uint32_t, spacker::Multiplier<>, 2>();
    EXPECT_EQ(val, 512 + 64 + 8);

    // Trying with a different scaling factor.
    val = spacker::max<uint32_t, spacker::Multiplier<8>, 0>();
    EXPECT_EQ(val, 128);

    val = spacker::max<uint32_t, spacker::Multiplier<8>, 1>();
    EXPECT_EQ(val, 16384 + 128);
}

TEST(PackMultiplierTest, AllOnes) {
    std::vector<uint8_t> sample(8, 1);
    auto packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    EXPECT_EQ(packed, std::vector<uint8_t>(4));
}

TEST(PackMultiplierTest, AllTwos) {
    std::vector<uint8_t> sample(4, 2);
    auto packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    EXPECT_EQ(packed, std::vector<uint8_t>(2, 0b00010001));
}

TEST(PackMultiplierTest, Nibble) {
    std::vector<uint8_t> sample{ 3, 4 };
    auto packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 1);
    EXPECT_EQ(packed[0], 0b00100011);
}

TEST(PackMultiplierTest, NibbleOverflow) {
    std::vector<uint8_t> sample{ 1, 3, 4 };
    auto packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b00000010);
    EXPECT_EQ(packed[1], 0b00110000); 

    // Something a bit more exotic - 5-bit integers!
    packed = spacker::pack_psip<false, spacker::Multiplier<5>>(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b00000000);
    EXPECT_EQ(packed[1], 0b10000110);
}

TEST(PackMultiplierTest, Byte) {
    std::vector<uint8_t> sample{ 10 };
    auto packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 1);
    EXPECT_EQ(packed[0], 0b10000001); // 00...010 = 1 -> +8+1 = 10

    sample[0] = 60;
    packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 1);
    EXPECT_EQ(packed[0], 0b10110011); // 110011 = 51 -> +8+1 = 60
}

TEST(PackMultiplierTest, ByteOverflow) {
    std::vector<uint8_t> sample{ 10, 20 };
    auto packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b10000001);
    EXPECT_EQ(packed[1], 0b10001011); // 1011 = 11 -> +8+1 = 20

    sample = std::vector<uint8_t>{ 1, 10, 20 };
    packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 3);
    EXPECT_EQ(packed[0], 0b00001000);
    EXPECT_EQ(packed[1], 0b00011000);
    EXPECT_EQ(packed[2], 0b10110000);

    // A number above the max_value naturally overflows.
    sample = std::vector<uint8_t>{ 255, 2 };
    packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b11001011);
    EXPECT_EQ(packed[1], 0b01100001);
}

TEST(PackMultiplierTest, ShortAgain) {
    std::vector<uint16_t> sample{ 10 };
    auto packed = spacker::pack_psip<false, spacker::Multiplier<> >(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 1);
    EXPECT_EQ(packed[0], 0b10000001);

    sample[0] = 100;
    packed = spacker::pack_psip<false, spacker::Multiplier<> >(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b11000001);
    EXPECT_EQ(packed[1], 0b10110000); // 11011 = 27 -> +72+1 = 100

    sample[0] = 500;
    packed = spacker::pack_psip<false, spacker::Multiplier<> >(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b11011010);
    EXPECT_EQ(packed[1], 0b10110000); // 110101011 = 427 -> +72+1 = 500

    sample[0] = 1000;
    packed = spacker::pack_psip<false, spacker::Multiplier<> >(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b11100001);
    EXPECT_EQ(packed[1], 0b10011111); // 1110011111 = 415 -> +584+1 = 1000

    sample[0] = 4000;
    packed = spacker::pack_psip<false, spacker::Multiplier<> >(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 2);
    EXPECT_EQ(packed[0], 0b11101101);
    EXPECT_EQ(packed[1], 0b01010111); // 111101010111 = 3415 -> +584+1 = 4000

    sample[0] = spacker::min<uint16_t, spacker::Multiplier<>, 4>() + 1;
    packed = spacker::pack_psip<false, spacker::Multiplier<> >(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 3);
    EXPECT_EQ(packed[0], 0b11110000);
    EXPECT_EQ(packed[1], 0);
    EXPECT_EQ(packed[2], 0b00010000);
}

TEST(PackMultiplierTest, ShortOverflow) {
    std::vector<uint16_t> sample{ 1, 22, 2068 };
    auto packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 4);
    EXPECT_EQ(packed[0], 0b00001000); // 1, then the first half of 22
    EXPECT_EQ(packed[1], 0b11011110); // rest of 22, then the tag for 2068
    EXPECT_EQ(packed[2], 0b01011100);
    EXPECT_EQ(packed[3], 0b10110000); // end of 2068, padded with zeros.

    // Triggering the other overflow-handling clause.
    sample = std::vector<uint16_t>{ 500, 22, 2068 };
    packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 5);
    EXPECT_EQ(packed[0], 0b11011010); // start of 500
    EXPECT_EQ(packed[1], 0b10111000); // end of 500, start of 22
    EXPECT_EQ(packed[2], 0b11011110); // end of 22, start of 2068
    EXPECT_EQ(packed[3], 0b01011100);
    EXPECT_EQ(packed[4], 0b10110000); // end of 2068, padded with zeros.
}

TEST(PackMultiplierTest, Word) {
    std::vector<uint32_t> sample{ spacker::min<uint32_t, spacker::Multiplier<>, 5>() + 1 };
    auto packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 3);
    EXPECT_EQ(packed[0], 0b11111000);
    EXPECT_EQ(packed[1], 0);
    EXPECT_EQ(packed[2], 1);

    sample = std::vector<uint32_t>{ spacker::max<uint32_t, spacker::Multiplier<>, 5>() };
    packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 3);
    EXPECT_EQ(packed[0], 0b11111011);
    EXPECT_EQ(packed[1], 0b11111111);
    EXPECT_EQ(packed[2], 0b11111111);

    sample[0] = spacker::max<uint32_t, spacker::Multiplier<>, 4>();
    packed = spacker::pack_psip<false, spacker::Multiplier<> >(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 3);
    EXPECT_EQ(packed[0], 0b11110111);
    EXPECT_EQ(packed[1], 0b11111111);
    EXPECT_EQ(packed[2], 0b11110000);
}

TEST(PackMultiplierTest, WordOverflow) {
    std::vector<uint32_t> sample{ 1, spacker::min<uint32_t, spacker::Multiplier<>, 5>() + 1, 2, spacker::max<uint32_t, spacker::Multiplier<>, 5>() };
    auto packed = spacker::pack_psip<false, spacker::Multiplier<>>(sample.size(), sample.data());
    ASSERT_EQ(packed.size(), 7);
    EXPECT_EQ(packed[0], 0b00001111);
    EXPECT_EQ(packed[1], 0b10000000);
    EXPECT_EQ(packed[2], 0b00000000);
    EXPECT_EQ(packed[3], 0b00010001);
    EXPECT_EQ(packed[4], 0b11111011);
    EXPECT_EQ(packed[5], 0b11111111);
    EXPECT_EQ(packed[6], 0b11111111);
}

TEST(PackMultiplierTest, RleSimple) {
    std::vector<uint8_t> sample(100, 1);
    auto val = spacker::pack_psip<true, spacker::Multiplier<>>(sample.size(), sample.data());
    EXPECT_EQ(val.size(), 4);
    EXPECT_EQ(val[0], 0b00001111); // padded with 1's.
    EXPECT_EQ(val[1], 0b11111111); // rle flag
    EXPECT_EQ(val[2], 0b11000001); // remaining bits represent a value of 100
    EXPECT_EQ(val[3], 0b10110000); 

    std::fill(sample.begin(), sample.end(), 21);
    val = spacker::pack_psip<true, spacker::Multiplier<>>(sample.size(), sample.data());
    EXPECT_EQ(val.size(), 4);
    EXPECT_EQ(val[0], 0b10001100); // 21
    EXPECT_EQ(val[1], 0b11111111); // rle flag
    EXPECT_EQ(val[2], 0b11000001); // remaining bits represent a value of 100
    EXPECT_EQ(val[3], 0b10110000); 

    std::fill(sample.begin(), sample.end(), 100);
    val = spacker::pack_psip<true, spacker::Multiplier<>>(sample.size(), sample.data());
    EXPECT_EQ(val.size(), 5);
    EXPECT_EQ(val[0], 0b11000001); // 100
    EXPECT_EQ(val[1], 0b10111111); // padded with 1's.
    EXPECT_EQ(val[2], 0b11111111); // rle flag
    EXPECT_EQ(val[3], 0b11000001); // remaining bits represent a value of 100
    EXPECT_EQ(val[4], 0b10110000); 
}

TEST(PackMultiplierTest, RleComplicated) {
    std::vector<uint16_t> sample(21, 3);
    sample[0] = 4;
    sample.push_back(4);
    auto val = spacker::pack_psip<true, spacker::Multiplier<>>(sample.size(), sample.data());

    EXPECT_EQ(val.size(), 4);
    EXPECT_EQ(val[0], 0b00110010); // 4, then 3.
    EXPECT_EQ(val[1], 0b11111111); // rle flag
    EXPECT_EQ(val[2], 0b10001011); // 20
    EXPECT_EQ(val[3], 0b00110000); // the terminating 4

    // Atempting a multi-word example:
    sample = std::vector<uint16_t>{ 1, 50, 50, 50, 50, 50, 50, 50, 50, 4 };
    val = spacker::pack_psip<true, spacker::Multiplier<>>(sample.size(), sample.data());

    EXPECT_EQ(val.size(), 4);
    EXPECT_EQ(val[0], 0b00001010); // 1, then 50.
    EXPECT_EQ(val[1], 0b10011111); // rest of 50, padded with 1's.
    EXPECT_EQ(val[2], 0b11111111); // rle flag
    EXPECT_EQ(val[3], 0b01110011); // 8, and then the terminating 4.
}

TEST(PackMultiplierTest, RleAutoChoice) {
    // Doesn't even exceed the RLE marker length; this is directly deposited into the buffer.
    std::vector<uint16_t> sample(2, 2);
    auto val = spacker::pack_psip<true, spacker::Multiplier<>>(sample.size(), sample.data());
    EXPECT_EQ(val[0], 0b00010001); 

    // Cost is 8 (rle marker) + 4 (the value) + 4 (the side of the run length) + 4 (padding).
    // This favors RLE, but such a preference disappears if we have one less element.
    sample = std::vector<uint16_t>(6, 2);
    val = spacker::pack_psip<true, spacker::Multiplier<>>(sample.size(), sample.data());
    EXPECT_EQ(val.size(), 3);
    EXPECT_EQ(val[0], 0b00011111); // value, padded with 1's.
    EXPECT_EQ(val[1], 0b11111111); // RLE marker
    EXPECT_EQ(val[2], 0b01010000); // length

    sample.pop_back();
    val = spacker::pack_psip<true, spacker::Multiplier<>>(sample.size(), sample.data());
    EXPECT_EQ(val.size(), 3);
    EXPECT_EQ(val[0], 0b00010001);
    EXPECT_EQ(val[1], 0b00010001);
    EXPECT_EQ(val[2], 0b00010000);
}
