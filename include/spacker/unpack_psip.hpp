#ifndef SPACKER_UNPACK_HPP
#define SPACKER_UNPACK_HPP

#include <cstdint>
#include <vector>
#include <limits>
#include <array>
#include <algorithm>

#include "pack_psip.hpp"

namespace spacker {

template<class Bits, class Buffer>
inline void unpack_psip_step (int keep, Bits& bits, Buffer& buffer, int& preamble, int& remaining, int& at) {
    // If preamble = true AND keep = true, we have an extra bit in the
    // preamble, we increase the remaining count. Otherwise neither
    // bits nor remaining will change.
    bits[at] += preamble * keep;
    remaining <<= preamble * keep;

    // If preamble = false, we left-shift the existing buffer element.
    // If additionally keep = true, we set the least significant bit.
    buffer[at] <<= (1 - preamble);
    buffer[at] |= (1 - preamble) * keep;

    // If preamble = false, we subtract a bit. If we're at the end of
    // the preamble (i.e., preamble = true and keep = false), we
    // compute the actual number of remaining bits, by subtracting
    // the bits used in defining the preamble itself.
    remaining -= (1 - preamble) + preamble * (1 - keep) * (bits[at] + 1);

    // If keep = false, then the preamble ended (or we we never in it).
    // If remaining = 0, then we've finished the current integer and
    // are moving onto the preamble of the next integer. No need to 
    // worry about this exceeding 1 as remaining > 0 during the preamble. 
    preamble = preamble * keep + (remaining == 0);

    // If there are no more remaining bits, we must be moving to the
    // preamble of the next element. In that case, we set remaining = 1
    // so that later left-shifts have an effect.
    at += (remaining == 0); 
    remaining += (remaining == 0);

    return;
}

template<typename T>
void unpack_psip(size_t ni, const uint8_t* input, size_t no, T* output) {
    std::array<T, 8> buffer;
    std::fill_n(buffer.data(), buffer.size(), 0);
    std::array<int, 8> bits;
    std::fill_n(bits.data(), bits.size(), 0);

    std::array<T, 8> baseline;
    std::fill_n(baseline.data(), baseline.size(), 0);

    baseline[0] = min_value_psip<T, 0>();
    baseline[1] = min_value_psip<T, 1>();
    baseline[2] = min_value_psip<T, 2>();
    baseline[3] = min_value_psip<T, 3>();
    baseline[4] = min_value_psip<T, 4>();

    // TODO: clean this up
    constexpr int digits = std::numeric_limits<T>::digits;
    if constexpr(digits >= (1 << 4)) {
        baseline[5] = min_value_psip<T, 5>();
    }
    if constexpr(digits >= (1 << 5)) {
        baseline[6] = min_value_psip<T, 6>();
    }
    if constexpr(digits >= (1 << 6)) {
        baseline[7] = min_value_psip<T, 7>();
    }

    // A boolean flag indicating whether we're still in the preamble.
    // We use int for easier multiplications below. We start at 1
    // to support the situation where the first entry is a preamble;
    // if it isn't, then it'll get flipped to zero anyway.
    int preamble = 1; 

    // Remaining bits to process in the current integer. We set it to
    // 1 because that's what is expected at the start of a new integer. 
    int remaining = 1; 

    // Where are we with respect to the buffers?
    int at = 0; 

    for (size_t i = 0; i < ni; ++i, ++input) {
        auto val = *input;

        // Manually unrolled. 
        unpack_psip_step ((val & 0b10000000) != 0, bits, buffer, preamble, remaining, at);
        unpack_psip_step ((val & 0b01000000) != 0, bits, buffer, preamble, remaining, at);
        unpack_psip_step ((val & 0b00100000) != 0, bits, buffer, preamble, remaining, at);
        unpack_psip_step ((val & 0b00010000) != 0, bits, buffer, preamble, remaining, at);
        unpack_psip_step ((val & 0b00001000) != 0, bits, buffer, preamble, remaining, at);
        unpack_psip_step ((val & 0b00000100) != 0, bits, buffer, preamble, remaining, at);
        unpack_psip_step ((val & 0b00000010) != 0, bits, buffer, preamble, remaining, at);
        unpack_psip_step ((val & 0b00000001) != 0, bits, buffer, preamble, remaining, at);

        // Moving results to the output buffer.
        int bound = std::min(static_cast<size_t>(at), no);
        for (int i = 0; i < bound; ++i, ++output) {
            *output = buffer[i] + baseline[bits[i]];
        }
        no -= bound;

        // Resetting for the next byte.
        std::fill_n(buffer.data(), at, 0);
        std::fill_n(bits.data(), at, 0);
        if (at && at < buffer.size()) {
            std::swap(buffer[0], buffer[at]);
            std::swap(bits[0], bits[at]);
        } 
        at = 0;
    }

    return;
}

}

#endif
