#ifndef SPACKER_PACK_HPP
#define SPACKER_PACK_HPP

#include <cstdint>
#include <vector>
#include <limits>

/**
 * @file psip_pack.hpp
 *
 * @brief Implements the positive small integer packer.
 */

namespace spacker {

template<typename T, int bits>
inline constexpr T max_value_psip() {
    if constexpr(bits == 0) {
        return 1;
    } else {
        static_assert((1 << bits) <= std::numeric_limits<T>::digits);
        constexpr int shift = (1 << bits) - bits - 1; // a.k.a. 2^(bits) - bits - 1, where the 1 comes from the extra zero.
        constexpr T space = (static_cast<T>(1) << shift); 
        constexpr T previous = max_value_psip<T, bits - 1>();
        return previous + space;
    }
}

template<typename T, int bits>
inline constexpr T min_value_psip() {
    if constexpr(bits == 0) {
        return 1;
    } else {
        return max_value_psip<T, bits - 1>() + 1;
    }
}

template<bool rle = true, typename T>
std::vector<uint8_t> pack_psip (size_t n, const T* input) {
    size_t i = 0;
    uint8_t buffer = 0;
    constexpr int width = 8;
    int leftover = width;

    std::vector<uint8_t> output;
    output.reserve(n/10);

    while (i < n) {
        auto val = input[i];

        if (val <= max_value_psip<T, 0>()) {
            buffer <<= 1;
            --leftover; 
            if (leftover == 0) {
                output.push_back(buffer);
                leftover = width;
                buffer = 0;
            }

        } else if (val <= max_value_psip<T, 3>()) {
            // This is where most of the values below uint8_t's will go.
            int bits = 1;

            if (val <= max_value_psip<T, 1>()) {
                val -= min_value_psip<T, 1>();
            } else if (val <= max_value_psip<T, 2>()) {
                bits = 2;
                val -= min_value_psip<T, 2>();
            } else {
                bits = 3;
                val -= min_value_psip<T, 3>();
            }

            int required = (1 << bits);
            uint8_t signature = (static_cast<uint8_t>(1) << (bits + 1)) - 2;
            signature <<= required - bits - 1;
            signature |= val;

            if (required <= leftover) {
                buffer <<= required;
                buffer |= signature;
                leftover -= required;
                if (leftover == 0) {
                    output.push_back(buffer);
                    leftover = width;
                    buffer = 0;
                }
            } else {
                buffer <<= leftover;
                int overflow = required - leftover;
                auto first = (signature >> overflow);
                buffer |= first;
                output.push_back(buffer);
                buffer = 0;
                buffer |= signature ^ (first << overflow); // Effectively subtraction, as RHS can only be 1 when LHS is 1.
                leftover = width - overflow;
            }

        } else {
            // No chance of fitting in a single uint8_t.
            int bits = 4;

            if (val <= max_value_psip<typename std::conditional<(sizeof(uint16_t) <= sizeof(T)), T, uint16_t>::type, 4>()) {
                val -= min_value_psip<T, 4>();
            } else {
                if constexpr(std::numeric_limits<T>::digits >= 16) {
                    if (val <= max_value_psip<typename std::conditional<(sizeof(uint32_t) <= sizeof(T)), T, uint32_t>::type, 5>()) {
                        bits = 5;
                        val -= min_value_psip<T, 5>();
                    } else {
                        if constexpr(std::numeric_limits<T>::digits >= 32) {
                            if (val <= max_value_psip<typename std::conditional<(sizeof(uint64_t) <= sizeof(T)), T, uint64_t>::type, 6>()) {
                                bits = 6; 
                                val -= min_value_psip<T, 6>();
                            } else {
                                if constexpr(std::numeric_limits<T>::digits >= 64) {
                                    bits = 7;
                                    val -= min_value_psip<T, 7>();
                                }
                            }
                        }
                    }
                }
            }

            int siglen = bits + 1;
            uint8_t preamble = (siglen == width ? std::numeric_limits<uint8_t>::max() - 1 : (static_cast<uint8_t>(1) << siglen) - 2); 

            // Adding the preamble first.
            if (siglen <= leftover) {
                buffer <<= siglen;
                buffer |= preamble;
                leftover -= siglen;
                if (leftover == 0) {
                    output.push_back(buffer);
                    leftover = width;
                    buffer = 0;
                }
            } else {
                buffer <<= leftover;
                int overflow = siglen - leftover;
                auto first = (preamble >> overflow);
                buffer |= first;
                output.push_back(buffer);

                buffer = 0;
                buffer |= preamble ^ (first << overflow); 
                leftover = width - overflow;
            }

            // Adding the rest of the signature. This requires some fiddling
            // to account for potential differences in integer width.
            int required = (1 << bits) - siglen;
            constexpr int available = std::numeric_limits<T>::digits;

            while (required > leftover) {
                buffer <<= leftover;

                int shift = required - leftover; 
                auto first = (shift > available ? 0 : (val >> shift) & 0b11111111);
                buffer |= static_cast<uint8_t>(first);
                output.push_back(buffer);

                required -= leftover;
                buffer = 0;
                leftover = width;
                val ^= (first << shift); 
            }

            buffer <<= leftover - required;
            buffer |= val;
            leftover -= required;
        }
        
        ++i;
    }

    if (leftover != width) {
        buffer <<= leftover;
        output.push_back(buffer);
    }

    return output;
}

}

#endif
