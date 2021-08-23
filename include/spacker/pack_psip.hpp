#ifndef SPACKER_PACK_HPP
#define SPACKER_PACK_HPP

#include <cstdint>
#include <vector>
#include <limits>

#include "utils.hpp"
#include "Doubling.hpp"

/**
 * @file pack_psip.hpp
 *
 * @brief Implements the positive small integer packer.
 */

namespace spacker {

template<typename T, int max_bits, class Scheme, int start_bits>
inline void determine_bits(T& val, int& bits) {
    if constexpr(start_bits == max_bits) {
        bits = start_bits;        
        val -= min<T, Scheme, start_bits>();
    } else {
        if constexpr(supports<T, Scheme, start_bits>()) {
            if (val <= max<T, Scheme, start_bits>()) {
                bits = start_bits;
                val -= min<T, Scheme, start_bits>();
            } else {
                determine_bits<T, max_bits, Scheme, start_bits + 1>(val, bits);
            }
        } else {
            bits = start_bits;
            val -= min<T, Scheme, start_bits>();
        }
    }
    return;
}

template<class Scheme, typename T>
inline constexpr int min_bits_per_byte() {
    if constexpr(max<T, Scheme, 0>() == 1) {
        return 1;
    } else {
        return 0;
    }
}

template<class Scheme, typename T>
int pack_psip_inner(T val, int& leftover, uint8_t& buffer, std::vector<uint8_t>& output) {
    constexpr int width = 8;

    // Packed version is a single bit.
    if constexpr(max<T, Scheme, 0>() == 1) {
        if (val == 1) {
            buffer <<= 1;
            --leftover; 
            if (leftover == 0) {
                output.push_back(buffer);
                leftover = width;
                buffer = 0;
            }
            return 1;
        }
    }

    if constexpr(Scheme::max_bits_per_byte() >= 0) {
        // Packed version fits into 1 byte.
        if (val <= max<T, Scheme, Scheme::max_bits_per_byte()>()) {
            int bits;
            determine_bits<T, Scheme::max_bits_per_byte(), Scheme, min_bits_per_byte<Scheme, T>()>(val, bits);

            const int required = Scheme::width(bits);
            uint8_t signature = (static_cast<uint8_t>(1) << (bits + 1)) - 2;
            signature <<= required - bits - 1;
            signature |= val;

            // Can it fit into 1 byte, or do we have to split it across 2?
            if (required <= leftover) {
                if (required < width) {
                    buffer <<= required;
                    buffer |= signature;
                } else {
                    buffer = signature;
                }
                leftover -= required;
                if (leftover == 0) {
                    output.push_back(buffer);
                    leftover = width;
                    buffer = 0;
                }
            } else {
                // not possible for leftover == width here, otherwise
                // it would have fallen into the previous clause.
                buffer <<= leftover; 
                int overflow = required - leftover;
                auto first = (signature >> overflow);
                buffer |= first;
                output.push_back(buffer);

                buffer = 0;
                buffer |= signature ^ (first << overflow); // Effectively subtraction, as RHS can only be 1 when LHS is 1.
                leftover = width - overflow;
            }

            return required;
        }
    } 
    
    // No chance of fitting in a single uint8_t.
    int bits;
    determine_bits<T, 7, Scheme, Scheme::max_bits_per_byte() + 1>(val, bits);
    const int siglen = bits + 1;
    const uint8_t preamble = (siglen == width ? std::numeric_limits<uint8_t>::max() - 1 : (static_cast<uint8_t>(1) << siglen) - 2); 

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
    const int required = Scheme::width(bits);
    int remaining = required - siglen;
    constexpr int available = std::numeric_limits<T>::digits;

    // Note that remaining >= leftover; to get to this point, required must
    // be at least 16, siglen cannot be more than 8, so remaining must be
    // at least 8, and leftover is no more than 8. This ensures that the
    // remaining operations are sensible, e.g., remaining -= leftover.

    if (leftover < width) {
        buffer <<= leftover;
        remaining -= leftover; // this is the shift to be applied to obtain the 8 bits of interest.
        if (remaining < available) {
            auto of_interest = (val >> remaining) & 0b11111111;
            buffer |= static_cast<uint8_t>(of_interest);
        }
        output.push_back(buffer);
    } else {
        ; // if leftover = width, buffer should have already been set to 0.
    }

    // Looping through all additional full-size bytes occupied by the current value.
    while (remaining >= width) {
        remaining -= width; 
        if (remaining >= available) {
            output.push_back(0);
        } else {
            auto of_interest = (val >> remaining) & 0b11111111;
            output.push_back(static_cast<uint8_t>(of_interest));
        }
    } 

    // Clearing out the remaining bytes.
    if (remaining) {
        leftover = width - remaining;
        int shift = available - remaining;
        auto of_interest = (val << shift) >> shift;
        buffer = static_cast<uint8_t>(of_interest);
    } else {
        leftover = width;
        buffer = 0;
    }

    return required;
}

template<bool rle = true, class Scheme = Doubling<>, typename T>
std::vector<uint8_t> pack_psip (size_t n, const T* input) {
    size_t i = 0;
    uint8_t buffer = 0;
    constexpr int width = 8;
    int leftover = width;

    std::vector<uint8_t> output;
    output.reserve(n/10);

    while (i < n) {
        auto val = input[i];
        int required = pack_psip_inner<Scheme>(val, leftover, buffer, output);

        if constexpr(rle) {
            auto copy = i + 1;
            while (copy < n && val == input[copy]) {
                ++copy;
            }

            // Approximate cost-effectiveness check.
            bool use_rle = false;
            size_t count = copy - i;
            size_t naive_cost = required * count;
            size_t rle_cost = width + required;
            if (naive_cost > rle_cost) {

                // Exact cost-effectiveness check.
                int run_bits;
                size_t count_copy = count;
                determine_bits<size_t, 7, Scheme, 0>(count_copy, run_bits);
                rle_cost += Scheme::width(run_bits);
                if (leftover < width && leftover > 0) {
                    rle_cost += leftover;
                }

                if (naive_cost > rle_cost) {
                    if (leftover < width && leftover > 0) { 
                        // Padding the current buffer with 1's.
                        uint8_t mask = 1;
                        mask <<= leftover;
                        mask -= 1;

                        buffer <<= leftover;
                        buffer |= mask;
                        output.push_back(buffer);

                        leftover = width;
                        buffer = 0;
                    }

                    // Adding the RLE marker.
                    output.push_back(0b11111111);

                    // Adding the length.
                    pack_psip_inner<Scheme>(count, leftover, buffer, output);

                    i = copy;
                    use_rle = true;
                }
            }

            if (!use_rle) {
                while ((++i) != copy) {
                    pack_psip_inner<Scheme>(val, leftover, buffer, output);
                }
            }

        } else {
            ++i;
        }
    }

    if (leftover != width) {
        buffer <<= leftover;
        output.push_back(buffer);
    }

    return output;
}

}

#endif
