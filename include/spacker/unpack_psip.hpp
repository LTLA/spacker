#ifndef SPACKER_UNPACK_HPP
#define SPACKER_UNPACK_HPP

#include <cstdint>
#include <vector>
#include <limits>
#include <array>
#include <algorithm>

#include "utils.hpp"
#include "Doubling.hpp"

namespace spacker {

template<class Scheme, class Bits, class Buffer>
inline void unpack_psip_step (int keep, Bits& bits, Buffer& buffer, int& preamble, int& remaining, int& at) {
    // If preamble = true AND keep = true, we have an extra bit in the
    // preamble, we increase the remaining count. Otherwise neither
    // bits nor remaining will change.
    bits[at] += preamble * keep;
    Scheme::update_remaining(remaining, preamble * keep);

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
    // so that later left-shifts or multiplications have an effect.
    at += (remaining == 0); 
    remaining += (remaining == 0) * Scheme::init_remaining;

    return;
}

template<class Scheme, typename T>
std::array<T, 8> initialize_baseline() {
    std::array<T, 8> baseline;
    std::fill_n(baseline.data(), baseline.size(), 0);

    baseline[0] = min<T, Scheme, 0>();

    if constexpr(supports<T, Scheme, 0>()) {
        baseline[1] = min<T, Scheme, 1>();
    }

    if constexpr(supports<T, Scheme, 1>()) {
        baseline[2] = min<T, Scheme, 2>();
    }

    if constexpr(supports<T, Scheme, 2>()) {
        baseline[3] = min<T, Scheme, 3>();
    }

    if constexpr(supports<T, Scheme, 3>()) {
        baseline[4] = min<T, Scheme, 4>();
    }

    if constexpr(supports<T, Scheme, 4>()) {
        baseline[5] = min<T, Scheme, 5>();
    }

    if constexpr(supports<T, Scheme, 5>()) {
        baseline[6] = min<T, Scheme, 6>();
    }

    if constexpr(supports<T, Scheme, 6>()) {
        baseline[7] = min<T, Scheme, 7>();
    }

    return baseline;
}

template<class Scheme = Doubling<>, typename T>
void unpack_psip(size_t ni, const uint8_t* input, size_t no, T* output) {
    std::array<T, 8> buffer;
    std::fill_n(buffer.data(), buffer.size(), 0);
    std::array<int, 8> bits;
    std::fill_n(bits.data(), bits.size(), 0);
    auto baseline = initialize_baseline<Scheme, T>();

    // Rle-related equivalents; this needs to be duplicated to ensure that we
    // can successfully recover lengths greater than T's max value.
    std::array<size_t, 8> rle_buffer;
    std::fill_n(rle_buffer.data(), rle_buffer.size(), 0);
    auto rle_baseline = initialize_baseline<Scheme, size_t>();

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

    size_t i = 0;
    while (i < ni) {
        auto val = *input;

        if (preamble == 1 && val == 0b11111111) {
            // Rle mode; running through and extracting the length.
            preamble = 1;
            remaining = 1;
            bits[at] = 0;

            do {
                ++i;
                ++input;
                val = *input;

                unpack_psip_step<Scheme>((val & 0b10000000) != 0, bits, rle_buffer, preamble, remaining, at);
                unpack_psip_step<Scheme>((val & 0b01000000) != 0, bits, rle_buffer, preamble, remaining, at);
                unpack_psip_step<Scheme>((val & 0b00100000) != 0, bits, rle_buffer, preamble, remaining, at);
                unpack_psip_step<Scheme>((val & 0b00010000) != 0, bits, rle_buffer, preamble, remaining, at);
                unpack_psip_step<Scheme>((val & 0b00001000) != 0, bits, rle_buffer, preamble, remaining, at);
                unpack_psip_step<Scheme>((val & 0b00000100) != 0, bits, rle_buffer, preamble, remaining, at);
                unpack_psip_step<Scheme>((val & 0b00000010) != 0, bits, rle_buffer, preamble, remaining, at);
                unpack_psip_step<Scheme>((val & 0b00000001) != 0, bits, rle_buffer, preamble, remaining, at);

            } while (at == 0 && i != ni - 1);

            size_t len = rle_buffer[0] + rle_baseline[bits[0]];
            size_t extra = len - 1; // extra ones to add, beyond the value already added.
            std::fill(output, output + extra, *(output - 1)); // cloning
            output += extra;
            no -= extra;

            // Moving any other results to the output buffer.
            int bound = std::min(static_cast<size_t>(at), no);
            for (int b = 1; b < bound; ++b, ++output) {
                *output = rle_buffer[b] + baseline[bits[b]];
            }
            no -= bound - 1; // because we already processed the first.

            // Preparing for the next byte.
            std::fill_n(rle_buffer.data(), at, 0);
            std::fill_n(bits.data(), at, 0);
            if (at && at < buffer.size()) {
                buffer[0] = rle_buffer[at];
                rle_buffer[at] = 0;
                std::swap(bits[0], bits[at]);
            } 
            at = 0;

            ++i;
            ++input;

        } else {
            // Manually unrolled. 
            unpack_psip_step<Scheme>((val & 0b10000000) != 0, bits, buffer, preamble, remaining, at);
            unpack_psip_step<Scheme>((val & 0b01000000) != 0, bits, buffer, preamble, remaining, at);
            unpack_psip_step<Scheme>((val & 0b00100000) != 0, bits, buffer, preamble, remaining, at);
            unpack_psip_step<Scheme>((val & 0b00010000) != 0, bits, buffer, preamble, remaining, at);
            unpack_psip_step<Scheme>((val & 0b00001000) != 0, bits, buffer, preamble, remaining, at);
            unpack_psip_step<Scheme>((val & 0b00000100) != 0, bits, buffer, preamble, remaining, at);
            unpack_psip_step<Scheme>((val & 0b00000010) != 0, bits, buffer, preamble, remaining, at);
            unpack_psip_step<Scheme>((val & 0b00000001) != 0, bits, buffer, preamble, remaining, at);

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

            ++i;
            ++input;
        }
    }

    return;
}

}

#endif
